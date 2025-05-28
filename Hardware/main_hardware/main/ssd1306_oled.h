/**
 * @file ssd1306_oled.h
 * @brief SSD1306 OLED display driver (128x64, I2C) for ESP32-S3 using ESP-IDF.
 *
 * This header defines configuration macros and function prototypes for the SSD1306 OLED driver.
 * Customize the I2C configuration (pins, port, address) via the macros below.
 */
#pragma once

#include <stdint.h>  // for uint8_t

// I2C configuration macros (adjust these as needed for your hardware setup)
#define SSD1306_I2C_PORT       I2C_NUM_1          /**< I2C port number to use (ESP32 has I2C_NUM_0 and I2C_NUM_1) */
#define SSD1306_I2C_SDA_PIN    14                 /**< GPIO number for I2C SDA line */
#define SSD1306_I2C_SCL_PIN    13                 /**< GPIO number for I2C SCL line */
#define SSD1306_I2C_FREQ_HZ    400000             /**< I2C bus frequency (Hz) */
#define SSD1306_I2C_ADDRESS    0x3C               /**< I2C address of the SSD1306 display (usually 0x3C or 0x3D) */

// Display resolution and buffer settings
#define SSD1306_WIDTH          128                /**< OLED width in pixels */
#define SSD1306_HEIGHT         64                 /**< OLED height in pixels */
#define SSD1306_PAGE_COUNT     (SSD1306_HEIGHT / 8) /**< Number of 8-pixel tall pages (rows) */
#define SSD1306_BUFFER_SIZE    (SSD1306_WIDTH * SSD1306_PAGE_COUNT) /**< Display buffer size in bytes */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the SSD1306 OLED display (I2C).
 * 
 * Configures the I2C peripheral and sends initialization commands to the display.
 * Must be called once before using other SSD1306 functions.
 */
void ssd1306_init(void);

/**
 * @brief Clear the display buffer.
 * 
 * Sets all pixels in the internal buffer to off (0). Call ssd1306_refresh() to apply the clear on the display.
 */
void ssd1306_clear(void);

/**
 * @brief Draw a single character to the buffer.
 * 
 * @param x     Horizontal start position (0 to SSD1306_WIDTH-1).
 * @param page  Vertical page index (0 to SSD1306_PAGE_COUNT-1). Each page is 8 pixels tall.
 * @param c     The ASCII character to draw.
 * 
 * Draws the character `c` at the specified position in the buffer using a 5x7 font.
 * Only modifies the buffer; call ssd1306_refresh() to update the actual display.
 */
void ssd1306_draw_char(uint8_t x, uint8_t page, char c);

/**
 * @brief Draw a null-terminated string to the buffer.
 * 
 * @param x      Horizontal start position for the first character.
 * @param page   Vertical page index (0 to SSD1306_PAGE_COUNT-1) for the string.
 * @param str    The null-terminated string to draw.
 * 
 * Draws the string starting at the given position in the buffer. Characters that do not fit 
 * on the line (beyond width) will be ignored. Uses a 5x7 font with 1-pixel spacing between characters.
 * After drawing, call ssd1306_refresh() to update the display.
 */
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str);

/**
 * @brief Send the buffer content to the OLED display.
 * 
 * Transfers the internal buffer to the SSD1306 display over I2C, updating the visible image.
 * Should be called after any drawing functions (draw_char, draw_string, clear) to apply changes.
 */
void ssd1306_refresh(void);

#ifdef __cplusplus
}
#endif
