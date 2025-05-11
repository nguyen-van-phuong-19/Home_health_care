
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "HELLO_WORLD";

void app_main(void)
{
    // In vòng lặp, mỗi giây in ra “Hello, world!”
    while (1) {
        ESP_LOGI(TAG, "Hello, world!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
