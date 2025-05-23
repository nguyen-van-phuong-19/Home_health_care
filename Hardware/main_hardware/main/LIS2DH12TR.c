#include "LIS2DH12TR.h"


static const char *TAG = "LIS2DH12TR";


// Write a single register
esp_err_t lis2dh12_write_reg(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS2DH12_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Read multiple registers with auto-increment
esp_err_t lis2dh12_read_regs(uint8_t reg, uint8_t *buffer, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS2DH12_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg | 0x80, true);  // set MSB to enable auto-increment
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS2DH12_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, buffer, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buffer + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Initialize LIS2DH12: ODR=100Hz, High-res, ±2g, X/Y/Z enabled
esp_err_t lis2dh12_init(void) {
    esp_err_t ret;
    // CTRL_REG1 = 0x57 (ODR=100Hz, LPen=0, Zen=1,Yen=1,Xen=1)
    ret = lis2dh12_write_reg(REG_CTRL1, 0x57);
    if (ret != ESP_OK) return ret;
    // CTRL_REG4 = 0x08 (HR=1, FS=00±2g)
    ret = lis2dh12_write_reg(REG_CTRL4, 0x08);
    if (ret != ESP_OK) return ret;
    ESP_LOGI(TAG, "Initialized: ODR=100Hz, High-res, ±2g");
    return ESP_OK;
}

// Read raw 16-bit values from OUTX_L..OUTZ_H
esp_err_t lis2dh12_read_raw(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buf[6];
    esp_err_t ret = lis2dh12_read_regs(REG_OUTX_L, buf, 6);
    if (ret != ESP_OK) return ret;
    *x = (int16_t)((buf[1] << 8) | buf[0]);
    *y = (int16_t)((buf[3] << 8) | buf[2]);
    *z = (int16_t)((buf[5] << 8) | buf[4]);
    return ESP_OK;
}

// Process raw counts to vector m/s²
lis2dh12_vector_t data_processing_from_LIS2DH12TR(
    int16_t raw_x, int16_t raw_y, int16_t raw_z,
    uint8_t fs_bits, bool lp_en, bool hr_en
) {
    uint8_t res = lp_en ? 8 : (hr_en ? 12 : 10);
    raw_x >>= (16 - res);
    raw_y >>= (16 - res);
    raw_z >>= (16 - res);

    float sens_mg;
    switch(fs_bits) {
        case 0: sens_mg = lp_en ? 16 : (hr_en ? 1 : 4); break;
        case 1: sens_mg = lp_en ? 32 : (hr_en ? 2 : 8); break;
        case 2: sens_mg = lp_en ? 64 : (hr_en ? 4 : 16); break;
        default: sens_mg = lp_en ? 192 : (hr_en ? 12 : 48);
    }

    float x_g = raw_x * sens_mg * 0.001f;
    float y_g = raw_y * sens_mg * 0.001f;
    float z_g = raw_z * sens_mg * 0.001f;

    lis2dh12_vector_t v = {
        .x = x_g * GRAVITY_MS2,
        .y = y_g * GRAVITY_MS2,
        .z = z_g * GRAVITY_MS2
    };
    return v;
}

// Convenient read vector
esp_err_t lis2dh12_get_vector(lis2dh12_vector_t *vec) {
    int16_t rx, ry, rz;
    esp_err_t ret = lis2dh12_read_raw(&rx, &ry, &rz);
    if (ret != ESP_OK) return ret;
    *vec = data_processing_from_LIS2DH12TR(rx, ry, rz, 0, false, true);
    return ESP_OK;
}

