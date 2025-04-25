#include "ble_fc.h"


static const char *TAG = "BLE_FC";

static const struct ble_gap_adv_params adv_params = {
    .conn_mode = BLE_GAP_CONN_MODE_UND,
    .disc_mode = BLE_GAP_DISC_MODE_GEN,
};

// Biến toàn cục theo dõi kết nối
static ble_state_t ble_current_state = BLE_STATE_DISCONNECTED;
static bool         ble_connected     = false;
static uint16_t     ble_conn_handle   = BLE_HS_CONN_HANDLE_NONE;

// Handle của characteristic (đã có sẵn)
static uint16_t ble_char_handle;

static int
ble_app_gap_event(struct ble_gap_event *event, void *arg);

// GATT access callback: respond to read requests
static int gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const char *reply = "ESP32-S3-NimBLE";
    os_mbuf_append(ctxt->om, reply, strlen(reply));
    return 0;
}

// GATT service definition
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180A), // Device Information
        .characteristics = (struct ble_gatt_chr_def[]) {{
            .uuid = BLE_UUID16_DECLARE(0x2A29), // Manufacturer Name
            .access_cb = gatt_access_cb,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0
        }}
    }, {
        0
    }
};

// BLE sync callback: set name, add services, start advertising
static void ble_app_on_sync(void)
{
    ble_svc_gap_device_name_set("ESP32-S3-BLE");
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);
    ble_current_state = BLE_STATE_ADVERTISING;
    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                    &adv_params,
                    ble_app_gap_event,  // <-- dùng callback mới
                    NULL);
    ESP_LOGI(TAG, "Advertising started");
}

// GATT registration event: capture handles
static void ble_app_gatt_event(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_REGISTER_OP_SVC) {
        ESP_LOGI(TAG, "Service registered; handle=%d", ctxt->svc.handle);
    } else if (ctxt->op == BLE_GATT_REGISTER_OP_CHR) {
        ble_char_handle = ctxt->chr.val_handle;
        ESP_LOGI(TAG, "Characteristic registered; handle=%d", ble_char_handle);
    }
}

// NimBLE host task
static void ble_host_task(void *param)
{
    nimble_port_run();
}

// Initialize BLE-only (NimBLE) GATT server
esp_err_t ble_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nimble_port_init();                   // init controller & host
    ble_hs_cfg.reset_cb = NULL;
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    ble_hs_cfg.gatts_register_cb = ble_app_gatt_event;
    nimble_port_freertos_init(ble_host_task);
    ESP_LOGI(TAG, "ahdjvbsjdv");
    return ESP_OK;
}

// Deinitialize BLE
esp_err_t ble_deinit(void)
{
    ESP_LOGI(TAG, "Stopping BLE host");
    nimble_port_stop();
    nimble_port_deinit();
    return ESP_OK;
}

// Send notification over BLE
esp_err_t ble_send_notification(uint16_t conn_handle,
                                const void *data,
                                uint16_t len)
{
    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    int rc = ble_gatts_notify_custom(conn_handle,
                                     ble_char_handle,
                                     om);
    return rc == 0 ? ESP_OK : ESP_FAIL;
}

static int
ble_app_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            ble_connected     = true;
            ble_conn_handle   = event->connect.conn_handle;
            ble_current_state = BLE_STATE_CONNECTED;
            ESP_LOGI(TAG, "BLE connected; handle=%d", ble_conn_handle);
        } else {
            ESP_LOGE(TAG, "BLE connection failed; status=%d", event->connect.status);
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE disconnected; reason=%d", event->disconnect.reason);
        ble_connected     = false;
        ble_conn_handle   = BLE_HS_CONN_HANDLE_NONE;
        ble_current_state = BLE_STATE_DISCONNECTED;
        ble_gap_adv_start(
            BLE_OWN_ADDR_PUBLIC,
            NULL,
            BLE_HS_FOREVER,
            &adv_params,
            ble_app_gap_event,
            NULL
        );
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete");
        ble_current_state = BLE_STATE_DISCONNECTED;
        return 0;

    default:
        return 0;
    }
}

ble_state_t ble_get_state(void)
{
    return ble_current_state;
}

bool ble_is_connected(void)
{
    return ble_connected;
}

uint16_t ble_get_conn_handle(void)
{
    return ble_conn_handle;
}
