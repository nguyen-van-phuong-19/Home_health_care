#include "main.h"

#define LIS2DH12_TASK_STACK_SIZE    2048
#define MAX30102_TASK_STACK_SIZE    4096
#define LIS2DH12_TASK_PRIORITY      5
#define MAX30102_TASK_PRIORITY      5

// Shared I2C mutex to protect bus on SDA/SCL
static SemaphoreHandle_t i2c_mutex = NULL;

static StaticTask_t lis2dh12_tcb;
static StackType_t  lis2dh12_stack[LIS2DH12_TASK_STACK_SIZE];
static StaticTask_t max30102_tcb;
static StackType_t  max30102_stack[MAX30102_TASK_STACK_SIZE];

static void lis2dh12_task(void *arg);
static void max30102_task(void *arg);

static uint32_t red_block[BLOCK_SIZE];
static uint32_t ir_block[BLOCK_SIZE];
static int block_index = 0;
static float convolved_signal[CONVOLVED_SIZE];

void app_main(void)
{
    // 2) Khởi tạo LIS2DH12TR
    ESP_ERROR_CHECK(i2c_master_init());
    // ESP_ERROR_CHECK(max30102_init());
    ESP_ERROR_CHECK(lis2dh12_init());
    ESP_LOGI("MAIN", "I2C initialized");

    // Create I2C mutex (with priority inheritance)
    i2c_mutex = xSemaphoreCreateMutex();
    configASSERT(i2c_mutex);

    xTaskCreateStatic(
        lis2dh12_task,      // hàm task
        "lis2dh12_task",    // tên task
        LIS2DH12_TASK_STACK_SIZE,               // stack size (bytes)
        NULL,               // tham số truyền vào
        LIS2DH12_TASK_PRIORITY,
        lis2dh12_stack,
        &lis2dh12_tcb
    );

    xTaskCreateStatic(
        max30102_task,      // hàm task
        "max30102_task",    // tên task
        MAX30102_TASK_STACK_SIZE,               // stack size (bytes)
        NULL,               // tham số truyền vào
        MAX30102_TASK_PRIORITY,
        max30102_stack,
        &max30102_tcb
    );

}


static void lis2dh12_task(void *arg)
{
    lis2dh12_vector_t acc;
    for(;;) {
        if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (lis2dh12_get_vector(&acc) == ESP_OK) {
                printf("Accel [m/s^2]: X=%7.3f  Y=%7.3f  Z=%7.3f\n",
                    acc.x, acc.y, acc.z);
            } else {
                ESP_LOGE("MAIN", "LIS2DH12TR read failed");
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
    }
}


static void max30102_task(void *arg)
{
    uint32_t red_val, ir_val;
    int beats = 0;
    float duration_sec = 0.0f;
    uint32_t hr = 0;
    float spo2_avg = 0.0f;

    for(;;) {
        if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if(max30102_read_sample(&red_val, &ir_val) == ESP_OK) {
                red_block[block_index] = red_val;
                ir_block[block_index]  = ir_val;
                block_index++;
                
                if(block_index >= BLOCK_SIZE) {
                    // Offline FIR convolution trên block của kênh Red
                    offline_fir_convolve(red_block, convolved_signal, BLOCK_SIZE);
                    
                    // Với tốc độ mẫu 100Hz, refractory period là 600 ms tương đương 60 mẫu
                    beats = count_beats_in_convolved(convolved_signal, CONVOLVED_SIZE, SAMPLE_INTERVAL_MS/1000.0f, 60);
                    duration_sec = (BLOCK_DURATION_MS) / 1000.0f;  // 10 s
                    hr = (uint32_t)(((float)beats / duration_sec) * 60);
                    
                    spo2_avg = compute_spo2_block(red_block, ir_block, BLOCK_SIZE);
                    
                    ESP_LOGI("RESULT", "Block (10s) -> HR: %d bpm, SpO2: %.1f%%, Beats: %d",
                                (int)hr, spo2_avg, beats);
                    
                    // Reset index để block mới
                    block_index = 0;
                    for(int i = 0; i < BLOCK_SIZE; i++) {
                        red_block[i] = 0;
                        ir_block[i]  = 0;
                    }

                    xSemaphoreGive(i2c_mutex);
                    vTaskDelay(pdMS_TO_TICKS(10000)); // Chờ 10 giây trước khi thu thập block tiếp theo
                    continue;
                }
            } else {
                ESP_LOGE("DATA", "Error reading sample");
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));  // 1 Hz
    }

}