#ifndef ST7789_LCD_H
#define ST7789_LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Pin configuration for ST7789 connection (ESP32-S3 GPIO numbers)
#define ST7789_PIN_MOSI   12    // MOSI (SPI data out from ESP32-S3)
#define ST7789_PIN_SCLK   15    // SCLK (SPI clock)
#define ST7789_PIN_DC     9    // D/C (Data/Command control pin)
#define ST7789_PIN_RST    8    // RESET pin (active low reset of the display)
#define ST7789_PIN_BL     10    // Backlight control pin (could use PWM for brightness)
#define ST7789_PIN_CS     -1    // Chip Select pin for SPI (-1 if not used/tied low)

// Display resolution (modify if using a different ST7789 panel)
#define ST7789_WIDTH      240   // Pixel width of the display
#define ST7789_HEIGHT     240   // Pixel height of the display

// Offsets (if the display panel has memory offset, e.g., 240x240 usually 0,0; 
// 240x320 or 135x240 panels might require offsets)
#define ST7789_X_OFFSET   0
#define ST7789_Y_OFFSET   0

// Color definitions in 16-bit RGB565 format (optional convenience macros)
#define ST7789_COLOR_BLACK   0x0000
#define ST7789_COLOR_WHITE   0xFFFF
#define ST7789_COLOR_RED     0xF800
#define ST7789_COLOR_GREEN   0x07E0
#define ST7789_COLOR_BLUE    0x001F

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the ST7789 display (SPI bus, GPIOs, and LCD controller)
esp_err_t st7789_init(void);

// Low-level send functions
void st7789_send_cmd(uint8_t cmd);
void st7789_send_data(const uint8_t *data, int len);

// Set the active drawing window (pixel address range) on the display
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Basic drawing functions
void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7789_fill_screen(uint16_t color);

// Set display rotation (0 to 3, corresponding to 0°, 90°, 180°, 270°)
void st7789_set_rotation(uint8_t rotation);

// Text rendering functions using 5x8 font
void st7789_draw_char(uint16_t x, uint16_t y, char c, uint16_t color);
void st7789_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // ST7789_LCD_H
