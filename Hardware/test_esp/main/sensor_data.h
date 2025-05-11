
#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

esp_err_t sensor_data_init(void);

esp_err_t sensor_data_set_hr(uint8_t hr);
esp_err_t sensor_data_get_hr(uint8_t *out_hr);

esp_err_t sensor_data_set_spo2(float spo2);
esp_err_t sensor_data_get_spo2(float *out_spo2);

esp_err_t sensor_data_set_motion(float motion);
esp_err_t sensor_data_get_motion(float *out_motion);
esp_err_t sensor_data_get_all(uint8_t *out_hr, uint8_t *out_spo2, float *out_motion);

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif // SENSOR_DATA_H
