// File: ble_fc.c

#include "esp_err.h"
#include "esp_log.h"
#include "host/ble_gap.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include <string.h>
#include "host/ble_hs_adv.h"    // cho ble_hs_adv_parse_fields


static const char *TAG = "BLE_FC";

void start_one_shot_scan(uint32_t duration_ms);

// Tham số quét (scan) nhanh
static const struct ble_gap_disc_params disc_params = {
    .itvl           = BLE_GAP_SCAN_FAST_INTERVAL_MIN,  // khoảng cách giữa các lần quét
    .window         = BLE_GAP_SCAN_FAST_WINDOW,        // thời gian mở cửa quét
    .filter_policy  = BLE_HCI_SCAN_FILT_NO_WL,         // không lọc theo whitelist
    .passive        = 0,                               // active scan (yêu cầu scan response)
};

// Callback xử lý các sự kiện GAP, trong đó có sự kiện DISC (khi quét thấy quảng bá)
static int
ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
        case BLE_GAP_EVENT_DISC: {
            struct ble_hs_adv_fields fields;
            int rc = ble_hs_adv_parse_fields(
                &fields,
                event->disc.data,
                event->disc.length_data
            );
            if (rc != 0) {
                ESP_LOGE(TAG, "adv_parse_fields failed; rc=%d", rc);
                break;
            }

            if (fields.mfg_data_len > 0) {
                const uint8_t *mac = event->disc.addr.val;
                // ESP_LOGI(TAG,
                //          "Found MfgData from %02x:%02x:%02x:%02x:%02x:%02x, len=%u",
                //          mac[5], mac[4], mac[3],
                //          mac[2], mac[1], mac[0],
                //          fields.mfg_data_len);
                // ESP_LOG_BUFFER_HEX(
                //     TAG,
                //     fields.mfg_data,
                //     fields.mfg_data_len
                // );
            }
            break;
        }

        default:
            break;
    }
    return 0;
}

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
    adv_params.channel_map   = BLE_HCI_ADV_CHANMASK_DEF;
    adv_params.filter_policy= BLE_HCI_ADV_FILT_DEF;

    uint8_t own_addr_type;
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto error: %d", rc);
        return;
    }

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_gap_event_cb, NULL);
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
esp_err_t ble_app_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        return err;
    }
    ESP_ERROR_CHECK(err);

    nimble_port_init();
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb  = ble_on_sync;
    nimble_port_freertos_init(ble_app_task);
    return ESP_OK;
}

// Update manufacturer data payload dynamically
esp_err_t update_service_data(const uint8_t *data, uint8_t len) {
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
        return rc;
    } else {
        ESP_LOGI(TAG, "Manufacturer Data updated len=%d", len);
    }
 /* 2) Dừng quảng bá */
    rc = ble_gap_adv_stop();
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_stop rc=%d", rc);
        // Có thể vẫn tiếp tục, nhưng tốt nhất là return
        return rc;
    }

    /* 3) Khởi động lại quảng bá với adv_params cũ */
    rc = ble_gap_adv_start(
        BLE_OWN_ADDR_PUBLIC,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        ble_gap_event_cb,
        NULL
    );
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_start rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Advertising restarted with new MfgData");
    }

    /* 4) Quét 1 lần như trước */
    start_one_shot_scan(500);  // scan 500 ms
    return ESP_OK;
}

void send_sensor_data(uint8_t hr, uint8_t spo2, float motion) {
    uint8_t buf[1 + 1 + 4];  // 1 byte hr + 1 byte spo2 + 4 byte float
    buf[0] = hr;
    buf[1] = spo2;
    memcpy(&buf[2], &motion, sizeof(motion));  // ghi float little-endian

    // gọi hàm update_service_data đã có để up payload
    update_service_data(buf, sizeof(buf));
}


esp_err_t ble_deinit(void) {
    int rc;
    esp_err_t err;

    // 1) Stop advertising
    rc = ble_gap_adv_stop();
    if (rc) {
        ESP_LOGE(TAG, "ble_gap_adv_stop() failed; rc=%d", rc);
        return rc;
    } else {
        ESP_LOGI(TAG, "Advertising stopped");
    }

    // 2) Stop the NimBLE host thread
    rc = nimble_port_stop();
    if (rc) {
        ESP_LOGE(TAG, "nimble_port_stop() failed; rc=%d", rc);
        return rc;
    } else {
        ESP_LOGI(TAG, "NimBLE host stopped");
    }

    // 3) Deinitialize FreeRTOS task if bạn dùng nimble_port_freertos_init()
    nimble_port_freertos_deinit();
    ESP_LOGI(TAG, "NimBLE FreeRTOS deinitialized");

    // 4) Deinitialize NimBLE port (bao gồm cả controller/VHCI deinit)
    nimble_port_deinit();
    ESP_LOGI(TAG, "NimBLE port deinitialized");

    // 5) (Tùy chọn) Deinit NVS nếu bạn đã init nó trong ble_app_init()
    err = nvs_flash_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_deinit() failed; err=0x%x", err);
        return err;
    } else {
        ESP_LOGI(TAG, "NVS flash deinitialized");
    }
    return ESP_OK;
}


// 2) Thêm hàm on-demand scan:
void start_one_shot_scan(uint32_t duration_ms)
{
    int rc;
    uint8_t own_addr_type;

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto error: %d", rc);
        return;
    }

    rc = ble_gap_disc(
        own_addr_type,
        duration_ms,
        &disc_params,
        ble_gap_event_cb,
        NULL
    );
    if (rc) {
        ESP_LOGE(TAG, "One-shot scan failed; rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "One-shot scan started (%u ms)", (unsigned)duration_ms);
    }
}
