// wifi_pr.c
#include "wifi_pr.h"

static const char *TAG = "wifi_pr";

// Event group để chờ kết nối
EventGroupHandle_t wifi_event_group;
static int retry_count;

// Xử lý sự kiện Wi-Fi và IP
static void wifi_event_handler(void* arg, esp_event_base_t base,
                               int32_t id, void* data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            retry_count++;
            ESP_LOGW(TAG, "Retry %d/%d", retry_count, WIFI_MAX_RETRY);
        } else {
            ESP_LOGE(TAG, "Connect fail after %d tries", WIFI_MAX_RETRY);
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Got IP");
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(const char* ssid, const char* password)
{
    // 1. Init NVS, TCP/IP, event loop
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // 2. Init Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // 3. Đăng ký event handler
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler, NULL, NULL);

    // 4. Tạo event group
    wifi_event_group = xEventGroupCreate();
    retry_count = 0;

    // 5. Cấu hình SSID/Password
    wifi_config_t wc = {0};
    strncpy((char*)wc.sta.ssid, ssid, sizeof(wc.sta.ssid)-1);
    strncpy((char*)wc.sta.password, password, sizeof(wc.sta.password)-1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wc);
    esp_wifi_start();
    ESP_LOGI(TAG, "Connecting to %s", ssid);

    // 6. Chờ kết nối (10s)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                          WIFI_CONNECTED_BIT,
                                          pdFALSE, pdTRUE,
                                          pdMS_TO_TICKS(10000));
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected");
    } else {
        ESP_LOGE(TAG, "Timeout");
    }
}

void wifi_stop(void)
{
    esp_wifi_stop();
    ESP_LOGI(TAG, "Wi-Fi stopped");
}
