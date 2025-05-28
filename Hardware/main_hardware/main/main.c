#include "main.h"

static const char *TAG = "APP_MAIN";
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
    // esp_err_t err = mqtt_subscribe_topic(MQTT_TOPIC_HEART_RATE, 0);
    // if (err == ESP_OK) {
    //     ESP_LOGI(TAG, "Subscribed to demo/pub");
    // } else {
    //     ESP_LOGE(TAG, "Subscribe failed (%d)", err);
    // }
    // err = mqtt_subscribe_topic(MQTT_TOPIC_SPO2, 0);
    // if (err == ESP_OK) {
    //     ESP_LOGI(TAG, "Subscribed to demo/pub");
    // } else {
    //     ESP_LOGE(TAG, "Subscribe failed (%d)", err);
    // }
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

static inline float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

/* --------------------------------------------------------------------------
 *  Compute SpO2 for a block of samples
 *  Algorithm (Maxim reference):
 *    DC  = mean
 *    AC  = RMS-deviation
 *    R   = (AC_red/DC_red) / (AC_ir/DC_ir)
 *    If R > 1.0  ⇒  assume channels swapped → R = 1 / R
 *    SpO2 = -45.060·R² + 30.354·R + 94.845   (valid 0.3–1.2)
 *    Fallback tuyến tính nhẹ ngoài dải
 * -------------------------------------------------------------------------- */
float compute_spo2_block(uint32_t *red_data,
                         uint32_t *ir_data,
                         int length)
{
    if (length < 4)           // block quá ngắn
        return -1.0f;

    /* ---------- 1. DC (mean) ---------- */
    double red_sum = 0.0, ir_sum = 0.0;
    for (int i = 0; i < length; ++i) {
        red_sum += red_data[i];
        ir_sum  += ir_data[i];
    }
    const double red_dc = red_sum / length;
    const double ir_dc  = ir_sum  / length;
    if (red_dc < 1.0 || ir_dc < 1.0)
        return -1.0f;

    /* ---------- 2. AC (RMS-deviation) ---------- */
    double red_var = 0.0, ir_var = 0.0;
    for (int i = 0; i < length; ++i) {
        red_var += (red_data[i] - red_dc) * (red_data[i] - red_dc);
        ir_var  += (ir_data[i]  - ir_dc)  * (ir_data[i]  - ir_dc);
    }
    red_var /= length;
    ir_var  /= length;
    const double red_ac = sqrt(red_var);
    const double ir_ac  = sqrt(ir_var);
    if (ir_ac < 1e-6)
        return -1.0f;

    /* ---------- 3. Ratio R ---------- */
    double R = (red_ac / red_dc) / (ir_ac / ir_dc);

    /* Giả thuyết nhầm kênh: nếu R > 1 thì đảo lại */
    if (R > 1.0)
        R = 1.0 / R;

    R = clampf((float)R, 0.10f, 3.00f);

    /* ---------- 4. SpO2 ---------- */
    double spo2;
    if (R >= 0.3 && R <= 1.2) {
        spo2 = -45.060 * R * R + 30.354 * R + 94.845;
    } else {
        spo2 = 110.0 - 25.0 * R;
    }
    spo2 = clampf((float)spo2, 0.0f, 99.8f);
    return (float)spo2;
}
