// l80r_process.c
#include "l80r_process.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "L80R";
static SemaphoreHandle_t data_mutex = NULL;
static l80r_data_t s_data;

// Chuyển chuỗi NMEA sang độ thập phân
static double nmea_to_decimal(const char *str, char dir) {
    if (!str || (dir!='N'&&dir!='S'&&dir!='E'&&dir!='W')) return 0.0;
    double val = atof(str);
    int deg = (int)(val / 100);
    double min = val - deg * 100;
    double dec = deg + min / 60.0;
    if (dir == 'S' || dir == 'W') dec = -dec;
    return dec;
}

void l80r_init(void) {
    // Tạo mutex bảo vệ dữ liệu
    data_mutex = xSemaphoreCreateMutex();
    // Cấu hình UART
    const uart_config_t cfg = {
        .baud_rate = L80R_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(L80R_UART_NUM, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(L80R_UART_NUM, L80R_UART_TX_PIN, L80R_UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(L80R_UART_NUM, L80R_MAX_LINE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_flush(L80R_UART_NUM));
    ESP_LOGI(TAG, "L80-R Initialized!");
}

void l80r_set_data(const l80r_data_t *new_data) {
    if (data_mutex && new_data) {
        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10))) {
            memcpy(&s_data, new_data, sizeof(s_data));
            xSemaphoreGive(data_mutex);
        }
    }
}

void l80r_get_data(l80r_data_t *out_data) {
    if (data_mutex && out_data) {
        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10))) {
            memcpy(out_data, &s_data, sizeof(s_data));
            xSemaphoreGive(data_mutex);
        }
    }
}

void l80r_parse_nmea(const char *nmea) {
    // Chỉ parse GGA
    if (nmea[0] == '$' && (strncmp(nmea+1, "GNGGA",5)==0 || strncmp(nmea+1, "GPGGA",5)==0)) {
        l80r_data_t temp = {0};
        char buf[L80R_MAX_LINE];
        strncpy(buf, nmea, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        
        // Tách tokens
        char *tokens[12] = {0};
        int idx = 0;
        char *p = strtok(buf, ",");
        while (p && idx < 12) {
            tokens[idx++] = p;
            p = strtok(NULL, ",");
        }
        if (idx > 9) {
            // UTC
            strncpy(temp.utc_time, tokens[1], sizeof(temp.utc_time)-1);
            // kinh vĩ độ
            temp.latitude  = nmea_to_decimal(tokens[2], tokens[3][0]);
            temp.longitude = nmea_to_decimal(tokens[4], tokens[5][0]);
            // chất lượng fix
            temp.fix_quality     = atoi(tokens[6]);
            temp.num_satellites  = atoi(tokens[7]);
            temp.hdop            = atof(tokens[8]);
            temp.altitude        = atof(tokens[9]);
            ESP_LOGI(TAG, "GGA: UTC=%s, lat=%.6f, lon=%.6f, fix=%d, sats=%d, alt=%.1f",
                     temp.utc_time, temp.latitude, temp.longitude,
                     temp.fix_quality, temp.num_satellites, temp.altitude);
            l80r_set_data(&temp);
        }
    }
}

void l80r_task(void *arg) {
    uint8_t *buf = malloc(L80R_MAX_LINE);
    char line[L80R_MAX_LINE];
    int pos = 0;
    while (1) {
        int len = uart_read_bytes(L80R_UART_NUM, buf, L80R_MAX_LINE, pdMS_TO_TICKS(100));
        if (len > 0) {
            ESP_LOGI(TAG, "L80-R get data!");
            for (int i = 0; i < len; i++) {
                char c = (char)buf[i];
                if (c == '\r' || c == '\n') {
                    if (pos > 0) {
                        line[pos] = '\0';
                        ESP_LOGD(TAG, "RAW> %s", line);
                        l80r_parse_nmea(line);
                        pos = 0;
                    }
                } else if (pos < L80R_MAX_LINE-1) {
                    line[pos++] = c;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    free(buf);
}

// Trong app_main():
//    l80r_init();
//    xTaskCreate(l80r_task, "l80r_task", 4096, NULL, 5, NULL);
// Sau đó có thể gọi l80r_get_data(&out) để lấy dữ liệu mới nhất.
