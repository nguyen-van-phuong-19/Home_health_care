#include "sensor_data.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"
#include <math.h>
#include <stdint.h>

static const char *TAG = "SENSOR_DATA";

/** Trong bộ nhớ chỉ giữ giá trị mới nhất */
static uint8_t  s_hr = 0;
static float  s_spo2 = 0;
static float    s_motion = 0;

/** Tổng calories tích lũy từ đầu ngày */
static float     s_daily_calories_total = 0;

/** Thông tin người dùng – có thể dùng để tính calories */
static uint8_t   s_user_age    = 23;    // mặc định
static float     s_user_weight = 73.0f; // mặc định (kg)
/** 0 = nam, 1 = nữ */
static uint8_t   s_user_gender = 0;

static float   s_last_cal_hr     = 0;
static float   s_last_cal_motion = 0;

static float s_sleep_hours = 0.0f;

static bool is_new_day = false;

// Struct lưu NVS
typedef struct {
    float cal_hr;
    float cal_motion;
    float cal_total;
} calories_nvs_t;

/** Mutex bảo vệ truy xuất đồng thời */
static SemaphoreHandle_t s_mutex = NULL;

esp_err_t sensor_data_init(void)
{
    if (s_mutex) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) {
        ESP_LOGE(TAG, "Create mutex failed");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "sensor_data initialized");
    return ESP_OK;
}

esp_err_t sensor_data_set_hr(uint8_t hr)
{
    if (!s_mutex) return ESP_ERR_INVALID_STATE;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    s_hr = hr;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_hr(uint8_t *out_hr)
{
    if (!s_mutex || !out_hr) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    *out_hr = s_hr;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_set_spo2(float spo2)
{
    if (!s_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_spo2 = spo2;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t set_new_day(void){
  if (!is_new_day) {
    is_new_day = true;
  }
    return ESP_OK;
}

esp_err_t sensor_data_get_spo2(float *out_spo2)
{
    if (!s_mutex || !out_spo2) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_spo2 = s_spo2;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_set_motion(float motion)
{
    if (!s_mutex) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_motion = motion;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_motion(float *out_motion)
{
    if (!s_mutex || !out_motion) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_motion = s_motion;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_all(uint8_t *out_hr, float *out_spo2, float *out_motion)
{
    if (!s_mutex) {
        ESP_LOGE(TAG, "Module not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if (!out_hr || !out_spo2 || !out_motion) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    *out_hr     = s_hr;
    *out_spo2   = s_spo2;
    *out_motion = s_motion;
    xSemaphoreGive(s_mutex);

    return ESP_OK;
}


esp_err_t sensor_data_set_user_params(uint8_t age, float weight_kg, uint8_t gender)
{
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    s_user_age    = age;
    s_user_weight = weight_kg;
    s_user_gender = gender;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

static float calculate_calories_hr(uint8_t hr, uint8_t age, float weight_kg, uint8_t gender, float duration_min)
{
    // Ví dụ công thức (Men):  Calories/min = ( -55.0969 + 0.6309·HR + 0.1988·weight + 0.2017·age ) / 4.184
    // Ví dụ công thức (Women): Calories/min = ( -20.4022 + 0.4472·HR – 0.1263·weight + 0.074·age ) / 4.184
    if(hr <= 10){
      return 1.05f;
    }
    float cpm;
    if (gender == 0) {
        cpm = ( -55.0969f
                + 0.6309f * hr
                + 0.1988f * weight_kg
                + 0.2017f * age )
              / 4.184f;
    } else {
        cpm = ( -20.4022f
                + 0.4472f * hr
                - 0.1263f * weight_kg
                + 0.0740f * age )
              / 4.184f;
    }
    return cpm * duration_min;
}

static float calculate_calories_motion(float vector_sum,
                                       float duration_min,
                                       float weight_kg)
{
    // vector_sum: Σ||a|| (m/s²) đã tích luỹ trong duration_min phút
    // duration_min: khoảng thời gian (phút) mà bạn đã tích luỹ vector_sum
    // weight_kg: cân nặng người dùng (kg)

    // 1) Chuyển duration sang giây
    float total_time_s = duration_min * 60.0f;

    // 2) Gia tốc trung bình (m/s²)
    float avg_accel = vector_sum / total_time_s;

    // 3) Tính MET: MET = (avg_accel / g) * 3.5
    const float g = 9.81f;
    float mets = (avg_accel / g) * 3.5f;

    // 4) Thời gian tính calories theo giờ
    float hours = duration_min / 60.0f;

    // 5) Calories = MET × weight (kg) × hours
    return mets * weight_kg * hours;
}

esp_err_t sensor_data_update_daily_calories(uint8_t age,
                                            float weight_kg,
                                            uint8_t gender,
                                            float duration_min,
                                            float *out_daily_total)
{
    if (!out_daily_total) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t hr;
    float motion_vector_sum;
    // Lấy giá trị nhịp tim và vector_sum (magnitude tích luỹ) hiện tại
    if (sensor_data_get_hr(&hr)    != ESP_OK ||
        sensor_data_get_motion(&motion_vector_sum) != ESP_OK) {
        return ESP_FAIL;
    }

    // Tính calories từ nhịp tim (giữ nguyên như trước)
    float cal_hr = calculate_calories_hr(hr, age, weight_kg, gender, 1);
    sensor_data_set_calories_hr(cal_hr);

    // Tính calories từ chuyển động với vector_sum và duration_min
    float cal_motion = calculate_calories_motion(
                           motion_vector_sum,
                           duration_min,
                           weight_kg
                       );
    sensor_data_set_calories_motion(cal_motion);

    // Cộng dồn vào tổng ngày
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    s_daily_calories_total += (cal_hr + cal_motion);
    *out_daily_total = s_daily_calories_total;
    if (is_new_day) {
      s_daily_calories_total = 0;
      is_new_day = false;
    }
    xSemaphoreGive(s_mutex);

    ESP_LOGI(TAG,
             "Cal HR=%.2f kcal, Cal Motion=%.2f kcal, TotalDay=%.2f kcal",
             cal_hr, cal_motion, s_daily_calories_total);

    return ESP_OK;
}

esp_err_t sensor_data_get_daily_calories(float *out_total)
{
    if (!out_total) {
        return ESP_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    *out_total = s_daily_calories_total;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

// --- Setter/getter calories_hr ---
esp_err_t sensor_data_set_calories_hr(float cal_hr) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_last_cal_hr += cal_hr;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_calories_hr(float *out_cal_hr) {
    if (!out_cal_hr) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_cal_hr = s_last_cal_hr;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

// --- Setter/getter calories_motion ---
esp_err_t sensor_data_set_calories_motion(float cal_motion) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_last_cal_motion += cal_motion;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_calories_motion(float *out_cal_motion) {
    if (!out_cal_motion) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_cal_motion = s_last_cal_motion;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

// Ghi 3 giá trị calories vào NVS
esp_err_t sensor_data_save_calories_nvs(void) {
    calories_nvs_t cal = {
        .cal_hr     = s_last_cal_hr,
        .cal_motion = s_last_cal_motion,
        .cal_total  = s_daily_calories_total
    };
    nvs_handle_t h;
    esp_err_t err = nvs_open("calories", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, "cal_all", &cal, sizeof(cal));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

// Đọc 3 giá trị calories từ NVS
esp_err_t sensor_data_load_calories_nvs(void) {
    calories_nvs_t cal;
    size_t required = sizeof(cal);
    nvs_handle_t h;
    esp_err_t err = nvs_open("calories", NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Chưa có dữ liệu, khởi tạo về 0
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        s_last_cal_hr            = 0;
        s_last_cal_motion        = 0;
        s_daily_calories_total   = 0;
        xSemaphoreGive(s_mutex);
        return ESP_OK;
    } else if (err != ESP_OK) {
        return err;
    }
    err = nvs_get_blob(h, "cal_all", &cal, &required);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Key chưa được ghi, khởi tạo về 0
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        s_last_cal_hr            = 0;
        s_last_cal_motion        = 0;
        s_daily_calories_total   = 0;
        xSemaphoreGive(s_mutex);
        return ESP_OK;
    } else if (err != ESP_OK) {
        return err;
    }
    // Đọc thành công, gán lại biến bộ nhớ
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_last_cal_hr            = cal.cal_hr;
    s_last_cal_motion        = cal.cal_motion;
    s_daily_calories_total   = cal.cal_total;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}


esp_err_t sensor_data_set_sleep_hours(float slp_hr) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_sleep_hours = slp_hr;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t sensor_data_get_sleep_hours(float *out_slp_hr) {
    if (!out_slp_hr) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out_slp_hr = s_sleep_hours;
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}
