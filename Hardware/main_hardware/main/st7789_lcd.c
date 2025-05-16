#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "st7789_lcd.h"

// SPI device handle for the ST7789 display
static spi_device_handle_t st7789_spi;

// Current display width and height (may change with rotation)
static uint16_t curr_width = ST7789_WIDTH;
static uint16_t curr_height = ST7789_HEIGHT;
static uint8_t curr_rotation = 0;  // current rotation (0-3)

// 5x8 font (glcdfont) data for ASCII characters 0x20-0x7E (96 characters)
// Each character is 5 bytes (columns) of bit patterns, LSB = top pixel, 1 bit = pixel on.
static const uint8_t st7789_font[96][5] = {
    // 0x20 ' ' (space) through 0x7E '~'
    {0x00,0x00,0x00,0x00,0x00}, // ' ' (space)
    {0x00,0x00,0x5F,0x00,0x00}, // '!' 
    {0x00,0x07,0x00,0x07,0x00}, // '"' 
    {0x14,0x7F,0x14,0x7F,0x14}, // '#' 
    {0x24,0x2A,0x7F,0x2A,0x12}, // '$' 
    {0x23,0x13,0x08,0x64,0x62}, // '%' 
    {0x36,0x49,0x55,0x22,0x50}, // '&' 
    {0x00,0x05,0x03,0x00,0x00}, // ''' 
    {0x00,0x1C,0x22,0x41,0x00}, // '(' 
    {0x00,0x41,0x22,0x1C,0x00}, // ')' 
    {0x14,0x08,0x3E,0x08,0x14}, // '*' 
    {0x08,0x08,0x3E,0x08,0x08}, // '+' 
    {0x00,0x50,0x30,0x00,0x00}, // ',' 
    {0x08,0x08,0x08,0x08,0x08}, // '-' 
    {0x00,0x60,0x60,0x00,0x00}, // '.' 
    {0x20,0x10,0x08,0x04,0x02}, // '/' 
    {0x3E,0x51,0x49,0x45,0x3E}, // '0' 
    {0x00,0x42,0x7F,0x40,0x00}, // '1' 
    {0x72,0x49,0x49,0x49,0x46}, // '2' 
    {0x21,0x49,0x4D,0x4B,0x31}, // '3' 
    {0x18,0x14,0x12,0x7F,0x10}, // '4' 
    {0x27,0x45,0x45,0x45,0x39}, // '5' 
    {0x3C,0x4A,0x49,0x49,0x30}, // '6' 
    {0x01,0x71,0x09,0x05,0x03}, // '7' 
    {0x36,0x49,0x49,0x49,0x36}, // '8' 
    {0x06,0x49,0x49,0x29,0x1E}, // '9' 
    {0x00,0x36,0x36,0x00,0x00}, // ':' 
    {0x00,0x56,0x36,0x00,0x00}, // ';' 
    {0x08,0x14,0x22,0x41,0x00}, // '<' 
    {0x14,0x14,0x14,0x14,0x14}, // '=' 
    {0x00,0x41,0x22,0x14,0x08}, // '>' 
    {0x02,0x01,0x59,0x09,0x06}, // '?' 
    {0x3E,0x41,0x5D,0x59,0x4E}, // '@' 
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A' 
    {0x7F,0x49,0x49,0x49,0x36}, // 'B' 
    {0x3E,0x41,0x41,0x41,0x22}, // 'C' 
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D' 
    {0x7F,0x49,0x49,0x49,0x41}, // 'E' 
    {0x7F,0x09,0x09,0x09,0x01}, // 'F' 
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G' 
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H' 
    {0x00,0x41,0x7F,0x41,0x00}, // 'I' 
    {0x20,0x40,0x41,0x3F,0x01}, // 'J' 
    {0x7F,0x08,0x14,0x22,0x41}, // 'K' 
    {0x7F,0x40,0x40,0x40,0x40}, // 'L' 
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M' 
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N' 
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O' 
    {0x7F,0x09,0x09,0x09,0x06}, // 'P' 
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q' 
    {0x7F,0x09,0x19,0x29,0x46}, // 'R' 
    {0x46,0x49,0x49,0x49,0x31}, // 'S' 
    {0x01,0x01,0x7F,0x01,0x01}, // 'T' 
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U' 
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V' 
    {0x7F,0x20,0x18,0x20,0x7F}, // 'W' 
    {0x63,0x14,0x08,0x14,0x63}, // 'X' 
    {0x07,0x08,0x70,0x08,0x07}, // 'Y' 
    {0x61,0x51,0x49,0x45,0x43}, // 'Z' 
    {0x00,0x7F,0x41,0x41,0x00}, // '[' 
    {0x02,0x04,0x08,0x10,0x20}, // '\' (backslash)
    {0x00,0x41,0x41,0x7F,0x00}, // ']' 
    {0x04,0x02,0x01,0x02,0x04}, // '^' 
    {0x80,0x80,0x80,0x80,0x80}, // '_' (underline)
    {0x00,0x03,0x05,0x00,0x00}, // '`' 
    {0x20,0x54,0x54,0x54,0x78}, // 'a' 
    {0x7F,0x48,0x44,0x44,0x38}, // 'b' 
    {0x38,0x44,0x44,0x44,0x20}, // 'c' 
    {0x38,0x44,0x44,0x48,0x7F}, // 'd' 
    {0x38,0x54,0x54,0x54,0x18}, // 'e' 
    {0x08,0x7E,0x09,0x01,0x02}, // 'f' 
    {0x0C,0x52,0x52,0x52,0x3E}, // 'g' 
    {0x7F,0x08,0x04,0x04,0x78}, // 'h' 
    {0x00,0x44,0x7D,0x40,0x00}, // 'i' 
    {0x20,0x40,0x44,0x3D,0x00}, // 'j' 
    {0x7F,0x10,0x28,0x44,0x00}, // 'k' 
    {0x00,0x41,0x7F,0x40,0x00}, // 'l' 
    {0x7C,0x04,0x18,0x04,0x78}, // 'm' 
    {0x7C,0x08,0x04,0x04,0x78}, // 'n' 
    {0x38,0x44,0x44,0x44,0x38}, // 'o' 
    {0x7C,0x14,0x14,0x14,0x08}, // 'p' 
    {0x08,0x14,0x14,0x18,0x7C}, // 'q' 
    {0x7C,0x08,0x04,0x04,0x08}, // 'r' 
    {0x48,0x54,0x54,0x54,0x20}, // 's' 
    {0x04,0x3F,0x44,0x40,0x20}, // 't' 
    {0x3C,0x40,0x40,0x20,0x7C}, // 'u' 
    {0x1C,0x20,0x40,0x20,0x1C}, // 'v' 
    {0x3C,0x40,0x30,0x40,0x3C}, // 'w' 
    {0x44,0x28,0x10,0x28,0x44}, // 'x' 
    {0x0C,0x50,0x50,0x50,0x3C}, // 'y' 
    {0x44,0x64,0x54,0x4C,0x44}, // 'z' 
    {0x00,0x08,0x36,0x41,0x00}, // '{' 
    {0x00,0x00,0x7F,0x00,0x00}, // '|' 
    {0x00,0x41,0x36,0x08,0x00}, // '}' 
    {0x08,0x08,0x2A,0x1C,0x08}  // '~' 
};

// Internal helper: send a command or data over SPI. DC line must be set accordingly.
// (Exposed via st7789_send_cmd and st7789_send_data)
static esp_err_t st7789_spi_send(const uint8_t *buffer, size_t len, bool dc) {
    // Set D/C line: low for command, high for data
    gpio_set_level(ST7789_PIN_DC, dc ? 1 : 0);

    if (len == 0) {
        return ESP_OK;
    }
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;           // length in bits
    t.tx_buffer = buffer;
    // (Chip select is handled by spi_device_transmit if CS pin is configured in device config)
    esp_err_t ret = spi_device_transmit(st7789_spi, &t);
    return ret;
}

esp_err_t st7789_init(void) {
    esp_err_t ret;

    // Configure GPIOs for DC, RST, BL (and CS if used)
    gpio_set_direction(ST7789_PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(ST7789_PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(ST7789_PIN_BL, GPIO_MODE_OUTPUT);
    if (ST7789_PIN_CS >= 0) {
        gpio_set_direction(ST7789_PIN_CS, GPIO_MODE_OUTPUT);
        // If using CS, start with it high (unselected)
        gpio_set_level(ST7789_PIN_CS, 1);
    }

    // Reset the display via RST pin
    if (ST7789_PIN_RST >= 0) {
        gpio_set_level(ST7789_PIN_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(ST7789_PIN_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = ST7789_PIN_MOSI,
        .miso_io_num = -1,                // Not used
        .sclk_io_num = ST7789_PIN_SCLK,
        .quadwp_io_num = -1,              // Not used
        .quadhd_io_num = -1,              // Not used
        .max_transfer_sz = ST7789_WIDTH * ST7789_HEIGHT * 2 + 8  // max transfer size in bytes
    };
    // Use default SPI host (VSPI for ESP32, SPI2_HOST for ESP32-S3)
    spi_host_device_t host = SPI2_HOST;
    ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE("ST7789", "Failed to initialize SPI bus: %d", ret);
        return ret;
    }

    // Configure SPI device interface for ST7789
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // SPI clock 40 MHz (ST7789 max ~62.5MHz)
        .mode = 0,                         // SPI mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = (ST7789_PIN_CS >= 0 ? ST7789_PIN_CS : -1), // CS pin (-1 if not used)
        .queue_size = 7,                   // transaction queue size
        .pre_cb = NULL, .post_cb = NULL    // no pre/post callbacks
    };
    ret = spi_bus_add_device(host, &devcfg, &st7789_spi);
    if (ret != ESP_OK) {
        ESP_LOGE("ST7789", "Failed to add SPI device: %d", ret);
        return ret;
    }

    // Turn off backlight initially
    gpio_set_level(ST7789_PIN_BL, 0);

    // ** ST7789 LCD Initialization sequence (following datasheet commands) **
    // Software Reset
    uint8_t cmd;
    cmd = 0x01; // SWRESET
    st7789_spi_send(&cmd, 1, false);
    vTaskDelay(pdMS_TO_TICKS(150));  // delay 150 ms

    // Sleep Out
    cmd = 0x11; // SLPOUT
    st7789_spi_send(&cmd, 1, false);
    vTaskDelay(pdMS_TO_TICKS(150));  // delay 150 ms

    // Color Mode: 16-bit/pixel
    cmd = 0x3A; // COLMOD (Pixel format set)
    st7789_spi_send(&cmd, 1, false);
    uint8_t data = 0x55; // 16-bit (RGB565)
    st7789_spi_send(&data, 1, true);
    vTaskDelay(pdMS_TO_TICKS(10));   // delay 10 ms

    // Memory Data Access Control (orientation)
    cmd = 0x36; // MADCTL
    st7789_spi_send(&cmd, 1, false);
    data = 0x00; // row col exchange off, normal order (RGB order)
    st7789_spi_send(&data, 1, true);

    // Inversion ON (for most displays, ST7789 looks better with inversion)
    cmd = 0x21; // INVON
    st7789_spi_send(&cmd, 1, false);
    vTaskDelay(pdMS_TO_TICKS(10));   // delay 10 ms

    // ** (Optional) You can set Porch, Gate Control, VCOM, etc., if required by specific panel **

    // Display ON
    cmd = 0x29; // DISPON
    st7789_spi_send(&cmd, 1, false);
    vTaskDelay(pdMS_TO_TICKS(100));  // delay 100 ms

    // Clear screen to black after init
    st7789_fill_screen(ST7789_COLOR_BLACK);

    // Turn on backlight
    gpio_set_level(ST7789_PIN_BL, 1);

    return ESP_OK;
}

void st7789_send_cmd(uint8_t cmd) {
    // Send a single command byte
    st7789_spi_send(&cmd, 1, false);
}

void st7789_send_data(const uint8_t *data, int len) {
    // Send a data buffer 
    st7789_spi_send(data, len, true);
}

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Adjust for display offsets (if any)
    x0 += ST7789_X_OFFSET;
    x1 += ST7789_X_OFFSET;
    y0 += ST7789_Y_OFFSET;
    y1 += ST7789_Y_OFFSET;

    uint8_t cmd, buf[4];
    // Column Address Set (CASET)
    cmd = 0x2A;
    st7789_send_cmd(cmd);
    buf[0] = (x0 >> 8) & 0xFF;
    buf[1] = x0 & 0xFF;
    buf[2] = (x1 >> 8) & 0xFF;
    buf[3] = x1 & 0xFF;
    st7789_send_data(buf, 4);

    // Row Address Set (RASET)
    cmd = 0x2B;
    st7789_send_cmd(cmd);
    buf[0] = (y0 >> 8) & 0xFF;
    buf[1] = y0 & 0xFF;
    buf[2] = (y1 >> 8) & 0xFF;
    buf[3] = y1 & 0xFF;
    st7789_send_data(buf, 4);

    // Write to RAM (RAMWR)
    cmd = 0x2C;
    st7789_send_cmd(cmd);
}

void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    // Check bounds
    if (x >= curr_width || y >= curr_height) {
        return;
    }
    // Set window to this pixel
    st7789_set_window(x, y, x, y);
    // Prepare color bytes (MSB first)
    uint8_t data[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
    // Send pixel color
    st7789_send_data(data, 2);
}

void st7789_fill_screen(uint16_t color) {
    // Fill entire screen with given color
    uint32_t total_pixels = curr_width * curr_height;
    // Set window to entire screen
    st7789_set_window(0, 0, curr_width - 1, curr_height - 1);
    // Prepare a buffer of a decent chunk of pixels to send in bursts
    const size_t batch_pixels = 128;
    uint8_t color_buf[batch_pixels * 2];
    // Fill the buffer with the color data
    uint8_t high_byte = color >> 8;
    uint8_t low_byte = color & 0xFF;
    for (size_t i = 0; i < batch_pixels; ++i) {
        color_buf[2*i] = high_byte;
        color_buf[2*i + 1] = low_byte;
    }
    // Send in batches
    uint32_t pixels_sent = 0;
    while (pixels_sent < total_pixels) {
        size_t pixels_to_send = batch_pixels;
        if (pixels_sent + pixels_to_send > total_pixels) {
            pixels_to_send = total_pixels - pixels_sent;
        }
        st7789_send_data(color_buf, pixels_to_send * 2);
        pixels_sent += pixels_to_send;
    }
}

void st7789_set_rotation(uint8_t rotation) {
    // Set display rotation (0-3)
    curr_rotation = rotation & 0x03;
    uint8_t madctl = 0;
    uint16_t new_width = ST7789_WIDTH;
    uint16_t new_height = ST7789_HEIGHT;
    // ST7789 MADCTL bits: MY=0x80, MX=0x40, MV=0x20, RGB=0x00 (RGB order) or 0x08 (BGR order)
    switch (curr_rotation) {
        case 0: // No rotation
            madctl = 0x00;
            new_width = ST7789_WIDTH;
            new_height = ST7789_HEIGHT;
            break;
        case 1: // 90° rotation
            madctl = 0x60; // MV=1, MX=1
            new_width = ST7789_HEIGHT;
            new_height = ST7789_WIDTH;
            break;
        case 2: // 180° rotation
            madctl = 0xC0; // MX=1, MY=1
            new_width = ST7789_WIDTH;
            new_height = ST7789_HEIGHT;
            break;
        case 3: // 270° rotation
            madctl = 0xA0; // MV=1, MY=1
            new_width = ST7789_HEIGHT;
            new_height = ST7789_WIDTH;
            break;
    }
    // If your color appears swapped (blue <-> red), set the RGB bit (bit3) appropriately.
    // Here we assume RGB order (madctl bit3 = 0). For BGR, use madctl |= 0x08.

    // Send MADCTL command to set rotation
    uint8_t cmd = 0x36;
    st7789_send_cmd(cmd);
    st7789_send_data(&madctl, 1);

    // Update stored width and height
    curr_width = new_width;
    curr_height = new_height;
}

void st7789_draw_char(uint16_t x, uint16_t y, char c, uint16_t color) {
    if (c < 0x20 || c > 0x7F) {
        // If character not in font, replace with a space
        c = ' ';
    }
    if ((x >= curr_width) || (y >= curr_height)) {
        return;
    }
    // Each character is 5 columns and 8 rows
    const uint8_t *char_data = st7789_font[c - 0x20];
    for (int8_t col = 0; col < 5; col++) {
        uint8_t line = char_data[col];
        for (int8_t row = 0; row < 8; row++) {
            if (line & 0x1) { // LSB is the top pixel of the column
                st7789_draw_pixel(x + col, y + row, color);
            }
            line >>= 1;
        }
    }
    // (Optionally, one column spacing could be added here for character spacing)
}

void st7789_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color) {
    uint16_t orig_x = x;
    while (*str) {
        if (*str == '\n') {
            // Newline: move to next line
            x = orig_x;
            y += 8; // 8 pixels down for new line (font height)
        } else if (*str == '\r') {
            // Carriage return: skip (if needed)
            // Do nothing, or reset x to origin
            x = orig_x;
        } else {
            // Draw character and advance cursor
            st7789_draw_char(x, y, *str, color);
            x += 6; // move cursor right by 6 pixels (5 for char + 1 for space)
            if (x + 5 >= curr_width) {
                // If beyond right edge, wrap to next line
                x = orig_x;
                y += 8;
            }
        }
        str++;
        // Stop if going beyond the bottom of screen
        if (y + 7 >= curr_height) {
            break;
        }
    }
}
