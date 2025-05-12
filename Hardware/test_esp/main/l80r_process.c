// l80r_process.c
#include "l80r_process.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "L80R";
static SemaphoreHandle_t data_mutex = NULL;
static l80r_data_t s_data;

void l80r_init(void) {
    // Create mutex
    data_mutex = xSemaphoreCreateMutex();

    // Initialize UART
    const uart_config_t uart_config = {
        .baud_rate = L80R_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(L80R_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(L80R_UART_NUM, L80R_UART_TX_PIN, L80R_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(L80R_UART_NUM, L80R_MAX_LINE * 2, 0, 0, NULL, 0));
}

// Convert NMEA lat/lon string to decimal degrees
static double nmea_to_decimal(const char *str, char dir) {
    double val = atof(str);
    int degrees = (int)(val / 100);
    double minutes = val - (degrees * 100);
    double dec = degrees + minutes / 60.0;
    if (dir == 'S' || dir == 'W') dec = -dec;
    return dec;
}

void l80r_parse_nmea(const char *nmea) {
    if (strncmp(nmea + 3, "GGA", 3) == 0) {
        // Temporary data
        l80r_data_t temp = {0};
        strncpy(temp.utc_time, nmea + 7, 15);
        temp.utc_time[15] = '\0';
        char buf[L80R_MAX_LINE];
        strncpy(buf, nmea, sizeof(buf));
        char *token = strtok(buf, ",");
        int field = 0;
        while (token) {
            switch (field) {
                case 1: strncpy(temp.utc_time, token, sizeof(temp.utc_time)); break;
                case 2: temp.latitude = nmea_to_decimal(token, buf[field == 3]); break;
                case 3: /* latitude dir handled */ break;
                case 4: temp.longitude = nmea_to_decimal(token, buf[field == 5]); break;
                case 5: /* longitude dir handled */ break;
                case 6: temp.fix_quality = atoi(token); break;
                case 7: temp.num_satellites = atoi(token); break;
                case 8: temp.hdop = atof(token); break;
                case 9: temp.altitude = atof(token); break;
                default: break;
            }
            token = strtok(NULL, ",");
            field++;
        }
        ESP_LOGI(TAG, "Parsed GGA: lat=%.6f, lon=%.6f, fix=%d, sats=%d, alt=%.1f",
                 temp.latitude, temp.longitude,
                 temp.fix_quality, temp.num_satellites,
                 temp.altitude);
        // Update shared data
        l80r_set_data(&temp);
    }
}

void l80r_set_data(const l80r_data_t *new_data) {
    if (!data_mutex || !new_data) return;
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10))) {
        memcpy(&s_data, new_data, sizeof(s_data));
        xSemaphoreGive(data_mutex);
    }
}

void l80r_get_data(l80r_data_t *out_data) {
    if (!data_mutex || !out_data) return;
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10))) {
        memcpy(out_data, &s_data, sizeof(s_data));
        xSemaphoreGive(data_mutex);
    }
}

void l80r_task(void *arg) {
    uint8_t *buf = (uint8_t *)malloc(L80R_MAX_LINE);
    char line[L80R_MAX_LINE];
    int line_pos = 0;
    while (1) {
        int len = uart_read_bytes(L80R_UART_NUM, buf, L80R_MAX_LINE, pdMS_TO_TICKS(100));
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                char c = (char)buf[i];
                if (c == '\r' || c == '\n') {
                    if (line_pos > 0) {
                        line[line_pos] = '\0';
                        l80r_parse_nmea(line);
                        line_pos = 0;
                    }
                } else if (line_pos < L80R_MAX_LINE - 1) {
                    line[line_pos++] = c;
                }
            }
        }
    }
    free(buf);
}

// Usage in app_main:
// l80r_init();
// xTaskCreate(l80r_task, "l80r_task", 4096, NULL, 5, NULL);
// // Later, when you need data:
// l80r_data_t current;
// l80r_get_data(&current);
