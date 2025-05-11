// File: ble_fc.c

#include "ble_fc.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include <string.h>

static const char *TAG = "BLE_FC";

// 128-bit Service UUID (little-endian)
#define SVC_UUID_BYTES   \
    0xF0,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78
static ble_uuid128_t svc_uuid128 = BLE_UUID128_INIT(SVC_UUID_BYTES);
static const ble_uuid128_t *adv_uuids128[] = { &svc_uuid128 };

// Manufacturer Data buffer: Company ID (2 bytes) + payload (max 8 bytes)
#define COMPANY_ID 0xABCD
// Company ID vẫn 2 byte, payload tối đa 27 byte
#define MFG_PAYLOAD_MAX 27

static uint8_t mfg_data[2 + MFG_PAYLOAD_MAX] = {
    (uint8_t)(COMPANY_ID & 0xFF),
    (uint8_t)(COMPANY_ID >> 8)
    // phần payload sẽ ghi vào từ chỉ số 2
};

// Advertising parameters
static struct ble_gap_adv_params adv_params;

// BLE reset callback
static void ble_on_reset(int reason) {
    ESP_LOGE(TAG, "BLE reset; reason=%d", reason);
}

// BLE sync callback: configure advertising and scan response
static void ble_on_sync(void) {
    ESP_LOGI(TAG, "BLE stack synced; configuring advertising");
    int rc;

    // 1) Advertising packet: flags + service UUID
    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.flags                = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.uuids128             = (const ble_uuid128_t *)adv_uuids128;
    adv_fields.num_uuids128         = 1;
    adv_fields.uuids128_is_complete = 1;

    rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc) {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields error: %d", rc);
        return;
    }

    // 2) Scan response: manufacturer data
    struct ble_hs_adv_fields rsp_fields;
    memset(&rsp_fields, 0, sizeof(rsp_fields));
    rsp_fields.mfg_data     = mfg_data;
    rsp_fields.mfg_data_len = sizeof(mfg_data);

    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc) {
        ESP_LOGE(TAG, "ble_gap_adv_rsp_set_fields error: %d", rc);
        return;
    }

    // 3) Advertising parameters: non-connectable + general discoverable
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode    = BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode    = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min     = 0x20; // ~20ms
    adv_params.itvl_max     = 0x40; // ~40ms
    adv_params.channel_map  = 0;
    adv_params.filter_policy= BLE_HCI_ADV_FILT_DEF;

    uint8_t own_addr_type;
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto error: %d", rc);
        return;
    }

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, NULL, NULL);
    if (rc) {
        ESP_LOGE(TAG, "ble_gap_adv_start error: %d", rc);
    } else {
        ESP_LOGI(TAG, "Advertising started");
    }
}

// BLE host task
static void ble_app_task(void *param) {
    nimble_port_run();
    // Should not return
    for (;;) {}
}

// Initialize BLE: NVS, NimBLE port, callbacks, task
void ble_app_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nimble_port_init();
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb  = ble_on_sync;
    nimble_port_freertos_init(ble_app_task);
}

// Update manufacturer data payload dynamically
void update_service_data(const uint8_t *data, uint8_t len) {
    if (len > MFG_PAYLOAD_MAX) {
        len = MFG_PAYLOAD_MAX;
    }
    memcpy(&mfg_data[2], data, len);
    // update scan response fields
    struct ble_hs_adv_fields rsp_fields;
    memset(&rsp_fields, 0, sizeof(rsp_fields));
    rsp_fields.mfg_data     = mfg_data;
    rsp_fields.mfg_data_len = 2 + len;
    int rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc) {
        ESP_LOGE(TAG, "update_service_data error: %d", rc);
    } else {
        ESP_LOGI(TAG, "Manufacturer Data updated len=%d", len);
    }
}

void send_sensor_data(uint8_t hr, uint8_t spo2, float motion) {
    uint8_t buf[1 + 1 + 4];  // 1 byte hr + 1 byte spo2 + 4 byte float
    buf[0] = hr;
    buf[1] = spo2;
    memcpy(&buf[2], &motion, sizeof(motion));  // ghi float little-endian

    // gọi hàm update_service_data đã có để up payload
    update_service_data(buf, sizeof(buf));
}
