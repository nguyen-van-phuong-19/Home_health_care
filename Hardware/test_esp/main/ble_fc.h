#ifndef BLE_FC_H
#define BLE_FC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "nvs_flash.h"                   // NVS for BLE host
#include "nimble/nimble_port.h"          // NimBLE port core
#include "nimble/nimble_port_freertos.h" // NimBLE FreeRTOS support
#include "host/ble_hs.h"                 // NimBLE host APIs
#include "host/ble_gatt.h"               // NimBLE GATT definitions
#include "services/gap/ble_svc_gap.h"    // GAP service helpers
#include "services/gatt/ble_svc_gatt.h"  // GATT service helpers
#include "esp_log.h"
#include <stdbool.h>

extern uint16_t ble_hr_handle;
extern uint16_t ble_spo2_handle;
extern uint16_t ble_acc_handle;
extern uint16_t ble_gps_handle;

// Các UUID 128-bit bạn đã định nghĩa trước
extern const ble_uuid128_t chr_hr_uuid;
extern const ble_uuid128_t chr_spo2_uuid;
extern const ble_uuid128_t chr_acc_uuid;
extern const ble_uuid128_t chr_gps_uuid;

#define ADV_INTERVAL_MIN      0x20
#define ADV_INTERVAL_MAX      0x40

extern EventGroupHandle_t ble_event_group;
#define BLE_CONNECTED_BIT  BIT0

typedef enum {
    BLE_STATE_DISCONNECTED = 0,
    BLE_STATE_ADVERTISING,
    BLE_STATE_CONNECTED
} ble_state_t;

typedef void (*ble_conn_cb_t)(uint16_t conn_handle);
void ble_register_conn_cb(ble_conn_cb_t cb);


/**
 * Lấy trạng thái hiện tại của BLE
 */
ble_state_t ble_get_state(void);

/**
 * Kiểm tra xem BLE đã kết nối thành công chưa
 */
bool ble_is_connected(void);

/**
 * Trả về `conn_handle` của kết nối hiện tại (hoặc BLE_HS_CONN_HANDLE_NONE nếu chưa có)
 */
uint16_t ble_get_conn_handle(void);


esp_err_t ble_init(void);
esp_err_t ble_deinit(void);
esp_err_t ble_send_notification(uint16_t conn_handle,
                                uint16_t char_handle,
                                const void *data,
                                uint16_t len);




#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // BLE_FC_H