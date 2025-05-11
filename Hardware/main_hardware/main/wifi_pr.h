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

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_MAX_RETRY     5

// Biến event group được khởi tạo trong wifi_pr.c, có thể extern ra để test_esp.c sử dụng
extern EventGroupHandle_t wifi_event_group;

typedef void (*wifi_error_handler_t)(void);

// Thiết lập callback khi kết nối thất bại
void wifi_set_error_handler(wifi_error_handler_t cb);

// Khởi STA (client) với SSID/Password
void wifi_init_sta(const char* ssid, const char* password);

// Khởi SoftAP với SSID/Password (nếu cần)
void wifi_init_softap(const char* ssid, const char* password);

// Dừng Wi-Fi
void wifi_stop(void);

// Handler mặc định khi fail (có thể override bằng wifi_set_error_handler)
void on_wifi_fail(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_PR_H
