// File: main/ble_notify_example.c

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"

static const char *TAG = "BLE_NOTIFY";

// --- Globals ---
static uint8_t own_addr_type;
static struct ble_gap_adv_params adv_params;
static uint16_t ble_conn_handle = BLE_HS_CONN_HANDLE_NONE;

// Storage for characteristic handle (must provide address to NimBLE)
static uint16_t chr_val_handle;

// Counter to notify
static uint32_t notify_counter = 0;


// GATT access callback: trả về giá trị của notify_counter khi client đọc
static int
notify_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // append 4-byte counter (little-endian)
    int rc = os_mbuf_append(ctxt->om, &notify_counter, sizeof(notify_counter));
    return rc == 0 ? 0 : BLE_ATT_ERR_UNLIKELY;
}

// Định nghĩa GATT service + characteristic
static struct ble_gatt_svc_def gatt_svcs[] = {
    {
        // Primary service UUID 0xABCD
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xAEFB),

        .characteristics = (struct ble_gatt_chr_def[]) { {
            // Characteristic UUID 0xDCBA, supports READ & NOTIFY
            .uuid       = BLE_UUID16_DECLARE(0xDCBA),
            .access_cb  = notify_access_cb,
            .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            .val_handle = &chr_val_handle,           // <–– provide storage
        }, {
            0 // terminator
        } }
    },
    {
        0 // terminator
    },
};


// GAP event callback: xử lý connect/disconnect
static int
ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            ble_conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Client connected; handle=%d", ble_conn_handle);
        } else {
            ESP_LOGW(TAG, "Connection failed; status=%d", event->connect.status);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Client disconnected; reason=%d",
                 event->disconnect.reason);
        ble_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        // Restart advertising
        ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                         &adv_params, ble_gap_event_cb, NULL);
        break;

    default:
        break;
    }
    return 0;
}

// Callback khi host+controller NimBLE đã đồng bộ
static void
ble_app_on_sync(void)
{
    int rc;

    // 1) Infer address type (public/random)
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);

    // 2) Đăng ký GATT services
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ESP_LOGI(TAG, "Registered GATT service, chr handle=%d", chr_val_handle);

    // 3) Đặt tên thiết bị GAP
    rc = ble_svc_gap_device_name_set("ESP32S3_NOTIFY");
    assert(rc == 0);

    // 4) Cấu hình advertising packet
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    const char *name = "ESP32S3_NOTIFY";
    fields.name = (uint8_t*)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    ble_uuid16_t svc_uuid = BLE_UUID16_INIT(0xAEFB);
    fields.uuids16 = &svc_uuid;
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    assert(rc == 0);

    // 5) Thiết lập advertising params (connectable + general discoverable)
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    // Không cần set itvl_min/itvl_max, NimBLE dùng mặc định

    // 6) Bắt đầu advertise với callback sự kiện GAP
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_gap_event_cb, NULL);
    assert(rc == 0);

    ESP_LOGI(TAG, "Advertising (connectable) started");
}

// Task chạy NimBLE host loop
static void
ble_host_task(void *arg)
{
    nimble_port_run();
}

// Task gửi notification mỗi 10 giây
static void
notify_task(void *arg)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10s
        if (ble_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            notify_counter++;
            ble_gatts_chr_updated(chr_val_handle);
            ESP_LOGI(TAG, "Notified count %" PRIu32, notify_counter);
        }
    }
}

void
app_main(void)
{
    esp_err_t ret;

    // 1) Khởi tạo NVS (cần cho controller BLE)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 2) Giải phóng RAM cho Bluetooth Classic
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // 3) Init NimBLE host + controller
    ESP_ERROR_CHECK(nimble_port_init());

    // 4) Đăng ký callback sync để set up GAP/GATT/Advertise
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    // 5) Tạo task chạy NimBLE host loop
    nimble_port_freertos_init(ble_host_task);

    // 6) Tạo task gửi notification định kỳ
    xTaskCreate(notify_task, "notify", 4096, NULL, 5, NULL);
}
