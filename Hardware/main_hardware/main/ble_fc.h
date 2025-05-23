#ifndef BLE_FC_H
#define BLE_FC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "esp_err.h"

/**
 * @brief Initialize BLE advertising (non-connectable) with service UUID and manufacturer data.
 */
esp_err_t ble_app_init(void);
esp_err_t ble_deinit(void);
/**
 * @brief Update manufacturer data payload dynamically.
 * @param data Pointer to new payload bytes.
 * @param len  Length of payload (max 8 bytes).
 */
esp_err_t update_service_data(const uint8_t *data, uint8_t len);

void send_sensor_data(uint8_t hr, uint8_t spo2, float motion);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // BLE_FC_H
