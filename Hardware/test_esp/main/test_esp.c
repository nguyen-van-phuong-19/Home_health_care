#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

// I2C configuration cho ESP32-S3-N16R8 (sử dụng chân phù hợp với board của bạn)
#define I2C_MASTER_SDA_IO           6
#define I2C_MASTER_SCL_IO           7
#define I2C_MASTER_NUM              I2C_NUM_0
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

// Các thông số hiệu chuẩn cho thuật toán phát hiện nhịp và SpO2
#define PEAK_THRESHOLD 3000.0f  // Ngưỡng tối thiểu của filtered signal để xác định đỉnh
// Các hằng số hiệu chuẩn cho SpO2 (đề xuất dựa trên dữ liệu tham chiếu từ người khỏe mạnh)
#define CAL_A 115.0f
#define CAL_B 15.0f

//------------------------------------------------------------------------------
// I2C functions (sử dụng API của ESP-IDF)
//------------------------------------------------------------------------------
esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if(err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

esp_err_t max30102_write_reg(uint8_t reg_addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX30102_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t max30102_read_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX30102_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX30102_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1)
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

//------------------------------------------------------------------------------
// MAX30102 initialization (set vào SpO2 mode)
//------------------------------------------------------------------------------
esp_err_t max30102_init(void) {
    esp_err_t ret;
    ret = max30102_write_reg(REG_MODE_CONFIG, MODE_CONFIG_VALUE);
    if(ret != ESP_OK) {
         ESP_LOGE("MAX30102", "Error writing MODE_CONFIG");
         return ret;
    }
    ret = max30102_write_reg(REG_SPO2_CONFIG, SPO2_CONFIG_VALUE);
    if(ret != ESP_OK) {
         ESP_LOGE("MAX30102", "Error writing SPO2_CONFIG");
         return ret;
    }
    ret = max30102_write_reg(REG_LED1_PA, LED1_PA_VALUE);
    if(ret != ESP_OK) {
         ESP_LOGE("MAX30102", "Error writing LED1_PA");
         return ret;
    }
    ret = max30102_write_reg(REG_LED2_PA, LED2_PA_VALUE);
    if(ret != ESP_OK) {
         ESP_LOGE("MAX30102", "Error writing LED2_PA");
         return ret;
    }
    // Reset FIFO pointers
    ret = max30102_write_reg(REG_FIFO_WR_PTR, 0x00);
    if(ret != ESP_OK) return ret;
    ret = max30102_write_reg(REG_FIFO_RD_PTR, 0x00);
    if(ret != ESP_OK) return ret;
    ESP_LOGI("MAX30102", "Initialization complete");
    return ESP_OK;
}

//------------------------------------------------------------------------------
// Đọc mẫu từ FIFO: Mỗi mẫu gồm 6 byte (3 cho Red, 3 cho IR)
//------------------------------------------------------------------------------
esp_err_t max30102_read_sample(uint32_t *red, uint32_t *ir) {
    uint8_t fifoData[6];
    esp_err_t ret = max30102_read_reg(REG_FIFO_DATA, fifoData, 6);
    if(ret != ESP_OK) return ret;
    *red = (((uint32_t)fifoData[0] << 16) | ((uint32_t)fifoData[1] << 8) | fifoData[2]) & 0x03FFFF;
    *ir  = (((uint32_t)fifoData[3] << 16) | ((uint32_t)fifoData[4] << 8) | fifoData[5]) & 0x03FFFF;
    return ESP_OK;
}

//------------------------------------------------------------------------------
// Thu thập block dữ liệu: Lưu BLOCK_SIZE mẫu (1000 mẫu cho 10 giây)
//------------------------------------------------------------------------------
static uint32_t red_block[BLOCK_SIZE];
static uint32_t ir_block[BLOCK_SIZE];
static int block_index = 0;

//------------------------------------------------------------------------------
// Offline FIR convolution: thực hiện convolution trên toàn block
// Kết quả có length = BLOCK_SIZE - FIR_TAP_NUM + 1
//------------------------------------------------------------------------------
void offline_fir_convolve(uint32_t *input, float *output, int length) {
    int out_length = length - FIR_TAP_NUM + 1;
    for (int i = 0; i < out_length; i++) {
        float acc = 0.0f;
        for (int j = 0; j < FIR_TAP_NUM; j++) {
            acc += bp_coeff[j] * ((float)input[i+j]);
        }
        output[i] = acc;
    }
}

//------------------------------------------------------------------------------
// Phát hiện đỉnh trong tín hiệu đã qua convolution
// Một mẫu được coi là đỉnh nếu nó là local maximum trong cửa sổ +/-1 mẫu
// và vượt quá ngưỡng PEAK_THRESHOLD, với refractory period bằng 600 ms (tương đương 600/100 = 6 mẫu, ở đây có thể tăng lên 60 mẫu nếu cần)
//------------------------------------------------------------------------------
int count_beats_in_convolved(float *data, int length, float sample_interval_sec, int refractory_samples) {
    int beat_count = 0;
    int last_peak_index = -refractory_samples; // Khởi tạo để đảm bảo mẫu đầu tiên có thể được đếm
    // Duyệt từ 1 đến length-2 để kiểm tra local maximum
    for (int i = 1; i < length-1; i++) {
        if(data[i] > PEAK_THRESHOLD &&
           data[i] > data[i-1] && data[i] > data[i+1] &&
           (i - last_peak_index) >= refractory_samples) {
            beat_count++;
            last_peak_index = i;
        }
    }
    return beat_count;
}

//------------------------------------------------------------------------------
// Tính HR và SpO2 dựa trên toàn bộ block
// Đối với SpO2: tính DC (trung bình) và AC (=(max-min)/2) cho từng kênh, sau đó
// Tỉ số R = (AC_red/DC_red) / (AC_ir/DC_ir) và công thức: SpO2 = CAL_A - CAL_B * R
float compute_spo2_block(uint32_t *red_data, uint32_t *ir_data, int length) {
    uint32_t red_dc = 0, ir_dc = 0;
    uint32_t red_max = red_data[0], red_min = red_data[0];
    uint32_t ir_max = ir_data[0], ir_min = ir_data[0];
    for (int i = 0; i < length; i++) {
        red_dc += red_data[i];
        ir_dc  += ir_data[i];
        if(red_data[i] > red_max) red_max = red_data[i];
        if(red_data[i] < red_min) red_min = red_data[i];
        if(ir_data[i] > ir_max) ir_max = ir_data[i];
        if(ir_data[i] < ir_min) ir_min = ir_data[i];
    }
    red_dc /= length;
    ir_dc  /= length;
    
    float red_ac = ((float)red_max - (float)red_min) / 2.0f;
    float ir_ac = ((float)ir_max - (float)ir_min) / 2.0f;
    
    if(red_dc == 0 || ir_dc == 0 || ir_ac == 0)
        return 0.0f;
    
    float R = (red_ac / red_dc) / (ir_ac / ir_dc);
    float spo2 = CAL_A - CAL_B * R;
    if(spo2 > 100.0f) spo2 = 100.0f;
    if(spo2 < 0.0f) spo2 = 0.0f;
    return spo2;
}

//------------------------------------------------------------------------------
// Main application: Thu thập BLOCK_SIZE mẫu (10s), xử lý offline,
// tính HR và SpO2 từ dữ liệu block.
#define CONVOLVED_SIZE (BLOCK_SIZE - FIR_TAP_NUM + 1)
static float convolved_signal[CONVOLVED_SIZE];

void app_main(void) {
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_ERROR_CHECK(max30102_init());
    
    block_index = 0;
    
    while(1) {
        uint32_t red_val, ir_val;
        if(max30102_read_sample(&red_val, &ir_val) == ESP_OK) {
            red_block[block_index] = red_val;
            ir_block[block_index]  = ir_val;
            block_index++;
            
            if(block_index < BLOCK_SIZE) {
                vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
                continue;
            }
            
            // Khi đã thu thập đủ 1000 mẫu (10 s)
            // Offline FIR convolution trên block của kênh Red
            offline_fir_convolve(red_block, convolved_signal, BLOCK_SIZE);
            
            // Với tốc độ mẫu 100Hz, refractory period là 600 ms tương đương 60 mẫu
            int beats = count_beats_in_convolved(convolved_signal, CONVOLVED_SIZE, SAMPLE_INTERVAL_MS/1000.0f, 60);
            float duration_sec = (BLOCK_DURATION_MS) / 1000.0f;  // 10 s
            uint32_t hr = (uint32_t)(((float)beats / duration_sec) * 60);
            
            float spo2_avg = compute_spo2_block(red_block, ir_block, BLOCK_SIZE);
            
            ESP_LOGI("RESULT", "Block (10s) -> HR: %d bpm, SpO2: %.1f%%, Beats: %d",
                     (int)hr, spo2_avg, beats);
            
            // Reset index để block mới
            block_index = 0;
            for(int i = 0; i < BLOCK_SIZE; i++) {
                red_block[i] = 0;
                ir_block[i]  = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(10000)); // Chờ 10 giây trước khi thu thập block tiếp theo
        } else {
            ESP_LOGE("DATA", "Error reading sample");
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
        }
    }
}
