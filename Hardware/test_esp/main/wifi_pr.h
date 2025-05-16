// wifi_pr.h
#ifndef WIFI_PR_H
#define WIFI_PR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"

extern EventGroupHandle_t wifi_event_group;

extern const int WIFI_CONNECTED_BIT;
#define WIFI_MAX_RETRY     3

// Biến event group được khởi tạo trong wifi_pr.c, có thể extern ra để test_esp.c sử dụng

typedef struct {
    const char *ssid;
    const char *password;
} wifi_ap_t;


extern const wifi_ap_t wifi_list[];
extern const size_t    wifi_list_count;

typedef void (*wifi_error_handler_t)(void);

// Thiết lập callback khi kết nối thất bại
void wifi_set_error_handler(wifi_error_handler_t cb);

// Khởi STA (client) với SSID/Password
void wifi_init_sta(void);

// Khởi SoftAP với SSID/Password (nếu cần)
void wifi_init_softap(const char* ssid, const char* password);

bool wifi_try_connect_list(TickType_t timeout_ms);

// Dừng Wi-Fi
void wifi_stop(void);

// Handler mặc định khi fail (có thể override bằng wifi_set_error_handler)
void on_wifi_fail(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_PR_H
