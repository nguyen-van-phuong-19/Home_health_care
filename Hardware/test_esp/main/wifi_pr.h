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



#define WIFI_CONNECTED_BIT BIT0
#define WIFI_MAX_RETRY     5


// wifi_pr.h
#pragma once
typedef void (*wifi_error_handler_t)(void);
void wifi_set_error_handler(wifi_error_handler_t cb);
void wifi_init_sta(const char* ssid, const char* password);
void wifi_init_softap(const char* ssid, const char* password);
void wifi_stop(void);
void on_wifi_fail(void);




#ifdef __cplusplus
}
#endif
#endif // WIFI_PR_H