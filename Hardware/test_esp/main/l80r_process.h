// l80r_process.h
#ifndef L80R_PROCESS_H
#define L80R_PROCESS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdint.h>


// UART instance and pins for L80-R
#define L80R_UART_NUM      UART_NUM_1
#define L80R_UART_TX_PIN   (GPIO_NUM_4)
#define L80R_UART_RX_PIN   (GPIO_NUM_5)
#define L80R_UART_BAUDRATE 9600

// Max NMEA line length
#define L80R_MAX_LINE      128

// Parsed GNSS data structure
typedef struct {
    char utc_time[16];       // hhmmss.ss string
    double latitude;         // decimal degrees
    double longitude;        // decimal degrees
    int    fix_quality;      // 0 = invalid, 1 = GPS fix, etc.
    int    num_satellites;   // satellites in view
    double hdop;             // Horizontal dilution of precision
    double altitude;         // meters above mean sea level
} l80r_data_t;

/**
 * @brief Initialize UART and mutex for L80-R module
 */
void l80r_init(void);

/**
 * @brief Task to read and process NMEA from L80-R
 * @param arg Unused
 */
void l80r_task(void *arg);

/**
 * @brief Parse a single NMEA sentence and update internal data
 * @param nmea Null-terminated NMEA sentence
 */
void l80r_parse_nmea(const char *nmea);

/**
 * @brief Set GNSS data manually (thread-safe)
 * @param new_data Pointer to new data to copy
 */
void l80r_set_data(const l80r_data_t *new_data);

/**
 * @brief Get current GNSS data (thread-safe)
 * @param out_data Pointer to user buffer to receive copy
 */
void l80r_get_data(l80r_data_t *out_data);

#ifdef __cplusplus
}
#endif

#endif // L80R_PROCESS_H
