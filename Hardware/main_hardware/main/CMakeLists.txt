idf_component_register(SRCS "main_hardware.c"
                    "main.c"
                    "LIS2DH12TR.c"
                    "wifi_pr.c"
                    "ble_fc.c"
                    "mqtt_cl.c"
                    "sensor_data.c"
                    "ssd1306_oled.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt                  # <— pull in esp_bt_main.h and friends
                        driver
                        esp_wifi
                        esp_netif
                        esp_event
                        nvs_flash
                        esp_system    # for system APIs
                        freertos      # for FreeRTOS headers
                        log           # for esp_log.h
                        mqtt
                        sdmmc 
                        esp_lcd
                        fatfs
                    )
