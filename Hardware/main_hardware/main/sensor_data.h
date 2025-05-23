
#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

esp_err_t sensor_data_init(void);

esp_err_t set_new_day(void);

esp_err_t sensor_data_set_hr(uint8_t hr);
esp_err_t sensor_data_get_hr(uint8_t *out_hr);

esp_err_t sensor_data_set_spo2(float spo2);
esp_err_t sensor_data_get_spo2(float *out_spo2);

esp_err_t sensor_data_set_motion(float motion);
esp_err_t sensor_data_get_motion(float *out_motion);
esp_err_t sensor_data_get_all(uint8_t *out_hr, float *out_spo2, float *out_motion);

// Thiết lập thông tin cơ bản (nếu cần lưu lại để tính calories)
esp_err_t sensor_data_set_user_params(uint8_t age, float weight_kg, uint8_t gender);
// Lấy tổng calories đã tiêu hao từ đầu ngày
esp_err_t sensor_data_get_daily_calories(float *out_total);
// Cập nhật và lấy tổng calories: tự động gọi tính calo HR + motion rồi cộng dồn
esp_err_t sensor_data_update_daily_calories(uint8_t age,
                                            float weight_kg,
                                            uint8_t gender,
                                            float duration_min,
                                            float *out_daily_total);

// Lấy giá trị calories từ nhịp tim vừa tính
esp_err_t sensor_data_get_calories_hr(float *out_cal_hr);
// Thiết lập giá trị calories từ nhịp tim (nếu cần ghi đè)
esp_err_t sensor_data_set_calories_hr(float cal_hr);
// Lấy giá trị calories từ chuyển động vừa tính
esp_err_t sensor_data_get_calories_motion(float *out_cal_motion);
// Thiết lập giá trị calories từ chuyển động (nếu cần ghi đè)
esp_err_t sensor_data_set_calories_motion(float cal_motion);

// --- NVS persistence ---
// Ghi 3 giá trị calories (hr, motion, total) vào NVS
esp_err_t sensor_data_save_calories_nvs(void);
// Đọc 3 giá trị calories từ NVS, khôi phục vào biến bộ nhớ
esp_err_t sensor_data_load_calories_nvs(void);

esp_err_t sensor_data_set_sleep_hours(float slp_hr);

esp_err_t sensor_data_get_sleep_hours(float *out_slp_hr);

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif // SENSOR_DATA_H
