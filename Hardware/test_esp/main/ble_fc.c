#include "ble_fc.h"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"

static const char *TAG = "BLE_FC";

// Định nghĩa byte cho UUID 128-bit
#define SVC_UUID_BYTES   \
    0xF0,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78

#define HR_UUID_BYTES    \
    0xF1,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78

#define SPO2_UUID_BYTES  \
    0xF2,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78

#define ACC_UUID_BYTES   \
    0xF3,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78

#define GPS_UUID_BYTES   \
    0xF4,0xDE,0xBC,0x9A, \
    0x78,0x56,0x34,0x12, \
    0x34,0x12,0x78,0x56, \
    0x34,0x12,0x56,0x78



static const struct ble_gap_adv_params adv_params = {
    .conn_mode   = BLE_GAP_CONN_MODE_UND,
    .disc_mode   = BLE_GAP_DISC_MODE_GEN,
    .itvl_min    = BLE_GAP_ADV_FAST_INTERVAL1_MIN,
    .itvl_max    = BLE_GAP_ADV_FAST_INTERVAL2_MAX,
    .channel_map = BLE_GAP_ADV_DFLT_CHANNEL_MAP,
};


EventGroupHandle_t ble_event_group = NULL;

// Biến lưu callback do ứng dụng cung cấp
static ble_conn_cb_t _user_cb = NULL;

// Hàm cho phép ứng dụng truyền con trỏ hàm vào
void ble_register_conn_cb(ble_conn_cb_t cb) {
  _user_cb = cb;
}

// Biến toàn cục theo dõi kết nối
static ble_state_t ble_current_state = BLE_STATE_DISCONNECTED;
static bool         ble_connected     = false;
static uint16_t     ble_conn_handle   = BLE_HS_CONN_HANDLE_NONE;

// Handle của characteristic (đã có sẵn)
uint16_t ble_hr_handle   = BLE_HS_CONN_HANDLE_NONE;
uint16_t ble_spo2_handle = BLE_HS_CONN_HANDLE_NONE;
uint16_t ble_acc_handle  = BLE_HS_CONN_HANDLE_NONE;
uint16_t ble_gps_handle  = BLE_HS_CONN_HANDLE_NONE;


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
static const struct ble_gatt_svc_def gatt_svr_svcs[] = { {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID128_DECLARE(SVC_UUID_BYTES),

    .characteristics = (struct ble_gatt_chr_def[]) { {
        .uuid       = BLE_UUID128_DECLARE(HR_UUID_BYTES),
        .flags      = BLE_GATT_CHR_F_READ  |
                      BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &ble_hr_handle,
    }, {
        .uuid       = BLE_UUID128_DECLARE(SPO2_UUID_BYTES),
        .flags      = BLE_GATT_CHR_F_READ  |
                      BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &ble_spo2_handle,
    }, {
        .uuid       = BLE_UUID128_DECLARE(ACC_UUID_BYTES),
        .flags      = BLE_GATT_CHR_F_READ  |
                      BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &ble_acc_handle,
    }, {
        .uuid       = BLE_UUID128_DECLARE(GPS_UUID_BYTES),
        .flags      = BLE_GATT_CHR_F_READ  |
                      BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &ble_gps_handle,
    }, {
        0  // end of this characteristic list
    } },
  },
  {
    0  // end of service list
  },
};


// BLE sync callback: set name, add services, start advertising
static void ble_app_on_sync(void) {
    // 1) Đăng ký Name + GATT services như cũ
    ble_svc_gap_device_name_set("ESP32-S3-BLE");
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // quảng bá service UUID 128-bit
    fields.num_uuids128         = 1;
    fields.uuids128             = (const ble_uuid_t*[]){
        BLE_UUID128_DECLARE(SVC_UUID_BYTES)
    };
    fields.uuids128_is_complete = 1;

    // (tuỳ chọn) quảng bá tên
    const char *name = "ESP32-S3";
    fields.name            = (uint8_t*)name;
    fields.name_len        = strlen(name);
    fields.name_is_complete = 1;

    int rc = ble_gap_adv_set_fields(&fields);
    if (rc) ESP_LOGE(TAG, "adv_set_fields failed: %d", rc);

    rc = ble_gap_adv_start(
      BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
      &adv_params,
      ble_app_gap_event,
      NULL
    );
    if (rc) ESP_LOGE(TAG, "adv_start failed: %d", rc);
}


static void
ble_app_gatt_event(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    if (ctxt->op != BLE_GATT_REGISTER_OP_CHR) {
        return;
    }

    const ble_uuid_t *u = ctxt->chr.chr_def->uuid;
    uint16_t          h = ctxt->chr.val_handle;

    // So sánh trực tiếp với các UUID literal qua BLE_UUID128_DECLARE
    if (ble_uuid_cmp(u,
        BLE_UUID128_DECLARE( 0xF1,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,
                            0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x78 )
    ) == 0) {
        ble_hr_handle = h;
    }
    else if (ble_uuid_cmp(u,
        BLE_UUID128_DECLARE( 0xF2,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,
                            0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x78 )
    ) == 0) {
        ble_spo2_handle = h;
    }
    else if (ble_uuid_cmp(u,
        BLE_UUID128_DECLARE( 0xF3,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,
                            0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x78 )
    ) == 0) {
        ble_acc_handle = h;
    }
    else if (ble_uuid_cmp(u,
        BLE_UUID128_DECLARE( 0xF4,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,
                            0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x78 )
    ) == 0) {
        ble_gps_handle = h;
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
    ble_event_group = xEventGroupCreate();
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
                                uint16_t char_handle,
                                const void *data,
                                uint16_t len)
{
    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    int rc = ble_gatts_notify_custom(conn_handle,
                                     char_handle,
                                     om);
    return (rc == 0) ? ESP_OK : ESP_FAIL;
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
            if (_user_cb) _user_cb(event->connect.conn_handle);
            xEventGroupSetBits(ble_event_group, BLE_CONNECTED_BIT);
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
        if (_user_cb) _user_cb(BLE_HS_CONN_HANDLE_NONE);
        xEventGroupClearBits(ble_event_group, BLE_CONNECTED_BIT);
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



// int len = snprintf(payload, sizeof(payload),
//     "{\"user_id\":\"user123\",\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.2f}",
//     latitude, longitude, altitude);
// ble_send_notification(conn_handle, ble_gps_handle, payload, len);
