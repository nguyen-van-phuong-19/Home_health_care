// wifi_pr.c
#include "wifi_pr.h"

static const char *TAG = "wifi_pr";

// Event group để chờ kết nối
EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
static int retry_count;


const wifi_ap_t wifi_list[] = {
    { "102",   "11111111"   },
    { "nguyen_phuong", "00000000" },
};
const size_t wifi_list_count = sizeof(wifi_list)/sizeof(wifi_list[0]);

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

void wifi_init_sta(void)
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

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}

bool wifi_try_connect_list(TickType_t timeout_ms)
{
    for (size_t i = 0; i < wifi_list_count; ++i) {
        // 1) Clear bit trước khi thử
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

        retry_count = 1;
        // 2) Cấu hình SSID/PASS
        wifi_config_t cfg = { 0 };
        strncpy((char*)cfg.sta.ssid,     wifi_list[i].ssid,     sizeof(cfg.sta.ssid)-1);
        strncpy((char*)cfg.sta.password, wifi_list[i].password, sizeof(cfg.sta.password)-1);
        esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);

        // 3) Nếu đang connect rồi thì disconnect
        esp_wifi_disconnect();
        // 4) Gọi connect (dùng config vừa set)
        esp_wifi_connect();

        ESP_LOGI(TAG, "Trying to connect to '%s' …", wifi_list[i].ssid);

        // 5) Chờ event BIT trong timeout
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE, pdTRUE,
            pdMS_TO_TICKS(timeout_ms)
        );
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected to '%s'", wifi_list[i].ssid);
            return true;
        }
        ESP_LOGW(TAG, "Failed to connect to '%s'", wifi_list[i].ssid);
    }
    ESP_LOGE(TAG, "Không kết nối được AP nào!");
    return false;
}

void wifi_stop(void)
{
    esp_wifi_stop();
    ESP_LOGI(TAG, "Wi-Fi stopped");
}
