// l80r_process.h
#ifndef L80R_PROCESS_H
#define L80R_PROCESS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <stdint.h>

// UART và chân kết nối cho module L80-R
#define L80R_UART_NUM      UART_NUM_1
#define L80R_UART_TX_PIN   (GPIO_NUM_4)
#define L80R_UART_RX_PIN   (GPIO_NUM_5)
#define L80R_UART_BAUDRATE 9600
#define L80R_MAX_LINE      128

// Cấu trúc lưu trữ dữ liệu GNSS đã parse
typedef struct {
    char   utc_time[16];      // "hhmmss.ss"
    double latitude;          // độ thập phân
    double longitude;         // độ thập phân
    int    fix_quality;       // 0=invalid,1=GPS fix...
    int    num_satellites;    // số vệ tinh
    double hdop;              // độ chính xác ngang
    double altitude;          // độ cao (m)
} l80r_data_t;

/**
 * @brief Khởi tạo UART và mutex cho L80-R
 */
void l80r_init(void);

/**
 * @brief Task đọc dữ liệu NMEA từ L80-R và parse
 * @param arg Không sử dụng
 */
void l80r_task(void *arg);

/**
 * @brief Parse câu NMEA GGA và cập nhật dữ liệu nội bộ
 * @param nmea Chuỗi NMEA kết thúc '\0'
 */
void l80r_parse_nmea(const char *nmea);

/**
 * @brief Ghi đè dữ liệu GNSS (thread-safe)
 */
void l80r_set_data(const l80r_data_t *new_data);

/**
 * @brief Lấy dữ liệu GNSS hiện tại (thread-safe)
 */
void l80r_get_data(l80r_data_t *out_data);

#ifdef __cplusplus
}
#endif

#endif // L80R_PROCESS_H
