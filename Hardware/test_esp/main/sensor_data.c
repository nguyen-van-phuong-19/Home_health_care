#include "sensor_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "SENSOR_DATA";

/** Trong bộ nhớ chỉ giữ giá trị mới nhất */
static uint8_t  s_hr;
static float  s_spo2;
static float    s_motion;

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

esp_err_t sensor_data_get_all(uint8_t *out_hr, uint8_t *out_spo2, float *out_motion)
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
