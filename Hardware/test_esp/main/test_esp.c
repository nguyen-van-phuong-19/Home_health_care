#include "main.h"

static void lis2dh12_task(void *arg);
static void max30102_task(void *arg);

static uint32_t red_block[BLOCK_SIZE];
static uint32_t ir_block[BLOCK_SIZE];
static int block_index = 0;
static float convolved_signal[CONVOLVED_SIZE];

// void app_main(void) {
//     ESP_ERROR_CHECK(i2c_master_init());
//     ESP_ERROR_CHECK(max30102_init());
    
//     block_index = 0;
    
//     while(1) {
//         uint32_t red_val, ir_val;
//         if(max30102_read_sample(&red_val, &ir_val) == ESP_OK) {
//             red_block[block_index] = red_val;
//             ir_block[block_index]  = ir_val;
//             block_index++;
            
//             if(block_index < BLOCK_SIZE) {
//                 vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
//                 continue;
//             }
            
//             // Offline FIR convolution trên block của kênh Red
//             offline_fir_convolve(red_block, convolved_signal, BLOCK_SIZE);
            
//             // Với tốc độ mẫu 100Hz, refractory period là 600 ms tương đương 60 mẫu
//             int beats = count_beats_in_convolved(convolved_signal, CONVOLVED_SIZE, SAMPLE_INTERVAL_MS/1000.0f, 60);
//             float duration_sec = (BLOCK_DURATION_MS) / 1000.0f;  // 10 s
//             uint32_t hr = (uint32_t)(((float)beats / duration_sec) * 60);
            
//             float spo2_avg = compute_spo2_block(red_block, ir_block, BLOCK_SIZE);
            
//             ESP_LOGI("RESULT", "Block (10s) -> HR: %d bpm, SpO2: %.1f%%, Beats: %d",
//                      (int)hr, spo2_avg, beats);
            
//             // Reset index để block mới
//             block_index = 0;
//             for(int i = 0; i < BLOCK_SIZE; i++) {
//                 red_block[i] = 0;
//                 ir_block[i]  = 0;
//             }
//             vTaskDelay(pdMS_TO_TICKS(10000)); // Chờ 10 giây trước khi thu thập block tiếp theo
//         } else {
//             ESP_LOGE("DATA", "Error reading sample");
//             vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
//         }
//     }
// }


void app_main(void)
{
    // 2) Khởi tạo LIS2DH12TR
    ESP_ERROR_CHECK(i2c_master_init());
    // ESP_ERROR_CHECK(max30102_init());
    ESP_ERROR_CHECK(lis2dh12_init());
    ESP_LOGI("MAIN", "I2C initialized");

    xTaskCreate(
        lis2dh12_task,      // hàm task
        "lis2dh12_task",    // tên task
        2048,               // stack size (bytes)
        NULL,               // tham số truyền vào
        tskIDLE_PRIORITY+1, // độ ưu tiên
        NULL                // handle (ko cần giữ)
    );

    xTaskCreate(
        max30102_task,      // hàm task
        "max30102_task",    // tên task
        2048,               // stack size (bytes)
        NULL,               // tham số truyền vào
        tskIDLE_PRIORITY+1, // độ ưu tiên
        NULL                // handle (ko cần giữ)
    );

}


static void lis2dh12_task(void *arg)
{
    lis2dh12_vector_t acc;
    while (1) {
        if (lis2dh12_get_vector(&acc) == ESP_OK) {
            printf("Accel [m/s^2]: X=%7.3f  Y=%7.3f  Z=%7.3f\n",
                   acc.x, acc.y, acc.z);
        } else {
            ESP_LOGE("MAIN", "LIS2DH12TR read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
    }
}


static void max30102_task(void *arg)
{
    uint32_t red_val, ir_val;
    if(max30102_read_sample(&red_val, &ir_val) == ESP_OK) {
        red_block[block_index] = red_val;
        ir_block[block_index]  = ir_val;
        block_index++;
        
        if(block_index < BLOCK_SIZE) {
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
        }else {
            
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
        }
    } else {
        ESP_LOGE("DATA", "Error reading sample");
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }
}