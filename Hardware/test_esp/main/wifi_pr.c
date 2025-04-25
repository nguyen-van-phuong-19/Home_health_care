#include "wifi_pr.h"

static const char *TAG = "wifi_pr";

// Event group to signal when connected
EventGroupHandle_t wifi_event_group;


static int wifi_retry_count;

// Optional user-defined callback on connection failure
typedef void (*wifi_error_handler_t)(void);
static wifi_error_handler_t error_handler = NULL;

void wifi_set_error_handler(wifi_error_handler_t cb)
{
    error_handler = cb;
}

static void mqtt_event_callback(const char *topic, const char *payload, int len) {
    ESP_LOGI(TAG, "Incoming on %-20s : %.*s", topic, len, payload);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi_retry_count < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            wifi_retry_count++;
            ESP_LOGW(TAG, "Retry %d/%d", wifi_retry_count, WIFI_MAX_RETRY);
        } else {
            ESP_LOGE(TAG, "Failed to connect after %d attempts", WIFI_MAX_RETRY);
            if (error_handler) error_handler();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(const char* ssid, const char* password)
{
    // 1. Khởi NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 2. TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // 3. Default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 4. Tạo default STA netif và đăng ký handler mặc định
    esp_netif_create_default_wifi_sta();

    // 5. Init Wi-Fi driver với cấu hình mặc định
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 6. Đăng ký callback cho sự kiện Wi-Fi và IP
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, NULL, NULL));

    // 7. Tạo EventGroup để chờ kết nối
    wifi_event_group = xEventGroupCreate();

    // 8. Cấu hình SSID/Password
    wifi_config_t wifi_config = { 0 };
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    // Đảm bảo strncpy không vượt quá kích thước
    strncpy((char*)wifi_config.sta.ssid,     ssid,     sizeof(wifi_config.sta.ssid)-1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password)-1);

    ESP_LOGI(TAG, "Configuring Wi-Fi SSID %s", ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // 9. Start Wi-Fi và connect
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Connecting to '%s'...", ssid);

    // 10. Chờ event kết nối hoặc timeout
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,    // không clear bit khi return
        pdTRUE,     // must wait all bits (chỉ có 1 bit thôi)
        pdMS_TO_TICKS(10000)  // chờ tối đa 10s
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP");
        esp_err_t err = mqtt_init(
            "mqtt://broker.emqx.io",   // Broker URI
            "esp32s3_example_client",     // Client ID
            mqtt_event_callback           // Callback nhận message
        );
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed mqtt_init (%d)", err);
            return;
        }
    } else {
        ESP_LOGE(TAG, "Failed to connect in time");
    }

}



void wifi_init_softap(const char* ssid, const char* password)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = { .ap = { .ssid_len = 0, .max_connection = 4, .authmode = WIFI_AUTH_WPA2_PSK } };
    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP '%s' started", ssid);
}

void wifi_stop(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG, "WiFi stopped");
}

// Default failure handler: stops Wi-Fi on persistent connection failure
void on_wifi_fail(void)
{
    ESP_LOGE(TAG, "Connection failed, invoking error handler: stopping WiFi");
    // wifi_stop();
}
