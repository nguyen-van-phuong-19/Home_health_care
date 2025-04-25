#ifndef LIS2DH12TR_HEADER_H
#define LIS2DH12TR_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "mqtt_cl.h"

// I2C address (SA0 = 1)
#define LIS2DH12_ADDR        0x19

// Register definitions
#define REG_CTRL1            0x20
#define REG_CTRL4            0x23
#define REG_OUTX_L           0x28  // auto-increment up to 0x2D

#define I2C_MASTER_SDA_IO           6
#define I2C_MASTER_SCL_IO           7
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000  // 100 kHz để ổn định
#define I2C_MASTER_TIMEOUT_MS       1000

#define GRAVITY_MS2          9.80665f

typedef struct {
    float x;  // m/s²
    float y;  // m/s²
    float z;  // m/s²
} lis2dh12_vector_t;

esp_err_t lis2dh12_write_reg(uint8_t reg, uint8_t data);
esp_err_t lis2dh12_read_regs(uint8_t reg, uint8_t *buffer, size_t len);
esp_err_t lis2dh12_init(void);
esp_err_t lis2dh12_read_raw(int16_t *x, int16_t *y, int16_t *z);
lis2dh12_vector_t data_processing_from_LIS2DH12TR(
    int16_t raw_x, int16_t raw_y, int16_t raw_z,
    uint8_t fs_bits, bool lp_en, bool hr_en
);
esp_err_t lis2dh12_get_vector(lis2dh12_vector_t *vec);

#ifdef __cplusplus
}
#endif  

#endif   // LIS2DH12TR_HEADER_H