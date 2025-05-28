#ifndef MAIN_HEADER_H   // 1) include‑guard
#define MAIN_HEADER_H

#ifdef __cplusplus   // 3) nếu dùng trong C++, giữ tên hàm đúng linkage C
extern "C" {
#endif


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "wifi_pr.h"
#include "mqtt_cl.h"
#include "ble_fc.h"
#include "LIS2DH12TR.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "sensor_data.h"
#include <stdint.h>


// I2C configuration cho ESP32-S3-N16R8 (sử dụng chân phù hợp với board của bạn)
#define I2C_MASTER_SDA_IO           14
#define I2C_MASTER_SCL_IO           13
#define I2C_MASTER_NUM              I2C_NUM_1
#define I2C_MASTER_FREQ_HZ          100000  // 100 kHz để ổn định
#define I2C_MASTER_TIMEOUT_MS       1000

// MAX30102 address and registers
#define MAX30102_ADDR               0x57
#define REG_MODE_CONFIG             0x09    // MODE_CONFIG: dùng 0x03 cho SpO2 mode
#define REG_SPO2_CONFIG             0x0A    // SPO2 configuration
#define REG_LED1_PA                 0x0C    // Red LED
#define REG_LED2_PA                 0x0D    // IR LED
#define REG_FIFO_WR_PTR             0x04
#define REG_FIFO_RD_PTR             0x06
#define REG_FIFO_DATA               0x07    // FIFO Data (6 bytes: 3 for Red, 3 for IR)

// MAX30102 configuration values
#define MODE_CONFIG_VALUE           0x03
#define SPO2_CONFIG_VALUE           0x27    // ví dụ: SPO2_ADC_RGE=0, SPO2_SR=0x03 (~400 sps), LED_PW=0x03 (411µs, 18-bit)
#define LED1_PA_VALUE               0x24
#define LED2_PA_VALUE               0x24

// Sampling parameters: 100 Hz, block 10 seconds (1000 samples)
#define SAMPLE_INTERVAL_MS          10  // 100 samples/s
#define BLOCK_DURATION_MS           10000  // 10 s
#define BLOCK_SIZE                  (BLOCK_DURATION_MS / SAMPLE_INTERVAL_MS)  // 1000 samples

// FIR Bandpass Filter parameters (51-tap filter)
// bp_coeff được thiết kế bằng Python với f_s = 100Hz, dải 0.5 – 5Hz, cửa sổ Hamming.
#define FIR_TAP_NUM                 51

// Các thông số hiệu chuẩn cho thuật toán phát hiện nhịp và SpO2
#define PEAK_THRESHOLD 3000.0f  // Ngưỡng tối thiểu của filtered signal để xác định đỉnh
// Các hằng số hiệu chuẩn cho SpO2 (đề xuất dựa trên dữ liệu tham chiếu từ người khỏe mạnh)
#define CAL_A 132.0f   
#define CAL_B 15.0f



//------------------------------------------------------------------------------
// Main application: Thu thập BLOCK_SIZE mẫu (10s), xử lý offline,
// tính HR và SpO2 từ dữ liệu block.
#define CONVOLVED_SIZE (BLOCK_SIZE - FIR_TAP_NUM + 1)


static const float bp_coeff[FIR_TAP_NUM] = {
    3.32486044e-04,  3.29430027e-04,  2.15175022e-04, -8.98882281e-05, -7.02767015e-04,
   -1.74996679e-03, -3.32883536e-03, -5.46483060e-03, -8.07303950e-03, -1.09325100e-02,
   -1.36806326e-02, -1.58320492e-02, -1.68227595e-02, -1.60758239e-02, -1.30810328e-02,
   -7.47781959e-03,  8.70886937e-04,  1.18258905e-02,  2.49498467e-02,  3.95222060e-02,
    5.45918953e-02,  6.90632328e-02,  8.18055781e-02,  9.17734311e-02,  9.81217282e-02,
    1.00301287e-01,  9.81217282e-02,  9.17734311e-02,  8.18055781e-02,  6.90632328e-02,
    5.45918953e-02,  3.95222060e-02,  2.49498467e-02,  1.18258905e-02,  8.70886937e-04,
   -7.47781959e-03, -1.30810328e-02, -1.60758239e-02, -1.68227595e-02, -1.58320492e-02,
   -1.36806326e-02, -1.09325100e-02, -8.07303950e-03, -5.46483060e-03, -3.32883536e-03,
   -1.74996679e-03, -7.02767015e-04, -8.98882281e-05,  2.15175022e-04,  3.29430027e-04,
    3.32486044e-04
};



esp_err_t i2c_master_init(void);
esp_err_t max30102_write_reg(uint8_t reg_addr, uint8_t data);
esp_err_t max30102_read_reg(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t max30102_init(void);
esp_err_t max30102_read_sample(uint32_t *red, uint32_t *ir);
void offline_fir_convolve(uint32_t *input, float *output, int length);
int count_beats_in_convolved(float *data, int length, float sample_interval_sec, int refractory_samples);
float compute_spo2_block(uint32_t *red_data, uint32_t *ir_data, int length);





#ifdef __cplusplus
}  // extern "C"
#endif

#endif
