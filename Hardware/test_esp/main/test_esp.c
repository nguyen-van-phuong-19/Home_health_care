#include "main.h"

#define LIS2DH12_TASK_STACK_SIZE    2048
#define MAX30102_TASK_STACK_SIZE    4096
#define LIS2DH12_TASK_PRIORITY      5
#define MAX30102_TASK_PRIORITY      5

static const char *TAG = "APP_MAIN";

// Shared I2C mutex to protect bus on SDA/SCL
static SemaphoreHandle_t i2c_mutex = NULL;

static StaticTask_t lis2dh12_tcb;
static StackType_t  lis2dh12_stack[LIS2DH12_TASK_STACK_SIZE];
static StaticTask_t max30102_tcb;
static StackType_t  max30102_stack[MAX30102_TASK_STACK_SIZE];
// cấu hình stack/TCB để tạo task tĩnh
// #define WIFI_WD_STACK_SIZE 4096
// static StaticTask_t   wifiWdTCB;
// static StackType_t    wifiWdStack[WIFI_WD_STACK_SIZE];

// static SemaphoreHandle_t mqtt_mutex = NULL;

static void lis2dh12_task(void *arg);
static void max30102_task(void *arg);
// static void wifi_watchdog_task(void *arg);
static void on_ble_conn(uint16_t h);

static uint32_t red_block[BLOCK_SIZE];
static uint32_t ir_block[BLOCK_SIZE];
static int block_index = 0;
static int convolved_index = 0;
static float convolved_signal[CONVOLVED_SIZE];
static float vector_sum = 0.0f;

static uint16_t conn_handle = 0xffff;  // sẽ được set khi có kết nối

void app_main(void)
{
    // wifi_init_sta("nguyen_phuong", "00000000");

    // 2) Khởi tạo LIS2DH12TR
    // ESP_ERROR_CHECK(i2c_master_init());
    // ESP_ERROR_CHECK(lis2dh12_init());
    // ESP_ERROR_CHECK(max30102_init());
    // ESP_LOGI("MAIN", "I2C initialized");

    // // Create I2C mutex (with priority inheritance)
    // i2c_mutex = xSemaphoreCreateMutex();
    // mqtt_mutex = xSemaphoreCreateMutex();
    // configASSERT(i2c_mutex);

    ble_register_conn_cb(on_ble_conn);
    if (ble_init() == ESP_OK) {
        ESP_LOGI(TAG, "ble_init OK");
    }
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        switch (ble_get_state()) {
        case BLE_STATE_DISCONNECTED:
            printf("BLE: disconnected\n");
            break;
        case BLE_STATE_ADVERTISING:
            printf("BLE: advertising\n");
            break;
        case BLE_STATE_CONNECTED:
            printf("BLE: connected (handle=%d)\n", ble_get_conn_handle());
            break;
        }
    }
    // xTaskCreateStatic(
    //     lis2dh12_task,      // hàm task
    //     "lis2dh12_task",    // tên task
    //     LIS2DH12_TASK_STACK_SIZE,               // stack size (bytes)
    //     NULL,               // tham số truyền vào
    //     LIS2DH12_TASK_PRIORITY,
    //     lis2dh12_stack,
    //     &lis2dh12_tcb
    // );

    // xTaskCreateStatic(
    //     max30102_task,      // hàm task
    //     "max30102_task",    // tên task
    //     MAX30102_TASK_STACK_SIZE,               // stack size (bytes)
    //     NULL,               // tham số truyền vào
    //     MAX30102_TASK_PRIORITY,
    //     max30102_stack,
    //     &max30102_tcb
    // );

    // xTaskCreateStatic(
    //     wifi_watchdog_task,         // entry fn
    //     "wifi_watchdog",            // name
    //     WIFI_WD_STACK_SIZE,         // stack depth (words)
    //     NULL,                       // param
    //     tskIDLE_PRIORITY + 1,       // priority
    //     wifiWdStack,                // stack buffer
    //     &wifiWdTCB                  // TCB buffer
    // );

}


static void lis2dh12_task(void *arg)
{
    lis2dh12_vector_t acc;
    char payload[256];
    for(;;) {
        if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (lis2dh12_get_vector(&acc) == ESP_OK) {
                vector_sum = vector_sum + sqrtf(acc.x * acc.x + acc.y * acc.y + acc.z * acc.z);
                convolved_index++;
                // printf("Accel [m/s^2]: X=%7.3f  Y=%7.3f  Z=%7.3f\n",
                //     acc.x, acc.y, acc.z);
                if(convolved_index >= 60 * 100){
                    EventBits_t bits = xEventGroupGetBits(ble_event_group);
                    // if((bits & WIFI_CONNECTED_BIT) == 0) {
                    if ((bits & BLE_CONNECTED_BIT)) {
                        int len = snprintf(payload, sizeof(payload),
                            "{\"user_id\":\"user123\",\"total_vector\":%.2f}",
                            vector_sum);
                        ble_send_notification(conn_handle, payload, len);
                    }
                    // }else {
                    // }
                    convolved_index = 0;
                }
            } else {
                ESP_LOGE("MAIN", "LIS2DH12TR read failed");
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // 100 Hz
    }
}


static void max30102_task(void *arg)
{
    uint32_t red_val, ir_val;
    int beats = 0;
    float duration_sec = 0.0f;
    uint32_t hr = 0;
    float spo2_avg = 0.0f;
    char payload[256];

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
                    
                    // ESP_LOGI("RESULT", "Block (10s) -> HR: %d bpm, SpO2: %.1f%%, Beats: %d",
                    //             (int)hr, spo2_avg, beats);
                    EventBits_t bits = xEventGroupGetBits(ble_event_group);
                    // if((bits & WIFI_CONNECTED_BIT) == 0) {
                    if ((bits & BLE_CONNECTED_BIT)) {
                        // ESP_LOGI("RESULT", "Block (10s) -> HR: %d bpm, SpO2: %.1f%%, Beats: %d",
                        //             (int)hr, spo2_avg, beats);
                        int len = snprintf(payload, sizeof(payload),
                            "{\"user_id\":\"user123\",\"bpm\":%d,}",
                            (int)hr);
                        ble_send_notification(conn_handle, payload, len);
                        len = snprintf(payload, sizeof(payload),
                            "{\"user_id\":\"user123\",\"percentage\":%.1f,}",
                            spo2_avg);
                        ble_send_notification(conn_handle, payload, len);
                    }
                    // }else {
                    // }
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


// static void wifi_watchdog_task(void *arg)
// {
//     EventGroupHandle_t eg = wifi_event_group;
//     bool wifi_was_up   = true;
//     bool ble_started   = false;

//     for (;;) {
//         EventBits_t bits = xEventGroupGetBits(eg);

//         if ((bits & WIFI_CONNECTED_BIT) == 0) {
//             // —— Wi-Fi DOWN ——
//             if (wifi_was_up) {
//                 // lần đầu phát hiện rớt, dọn dẹp MQTT + khởi BLE
//                 ESP_LOGW(TAG, "WiFi lost: deinit MQTT, init BLE");
//                 if (mqtt_deinit() == ESP_OK) {
//                     ESP_LOGI(TAG, "mqtt_deinit OK");
//                 }
//                 if (ble_init() == ESP_OK) {
//                     ESP_LOGI(TAG, "ble_init OK");
//                     ble_started = true;
//                 }
//                 wifi_was_up   = false;
//             }

//             // thử reconnect Wi-Fi
//             ESP_LOGI(TAG, "Watchdog: esp_wifi_connect()");
//             esp_err_t err = esp_wifi_connect();
//             if (err != ESP_OK) {
//                 ESP_LOGE(TAG, "esp_wifi_connect() failed: %d", err);
//             }

//         } else {
//             // —— Wi-Fi UP ——
//             if (!wifi_was_up) {
//                 // lần đầu phát hiện lên lại, dọn BLE + khởi MQTT
//                 ESP_LOGI(TAG, "WiFi reconnected: deinit BLE, init MQTT");
//                 if (ble_started && ble_deinit() == ESP_OK) {
//                     ESP_LOGI(TAG, "ble_deinit OK");
//                 }
//                 // Giả sử bạn muốn tự động tái-khởi MQTT:
//                 if (mqtt_init("mqtt://broker", "client_id", NULL) == ESP_OK) {
//                     ESP_LOGI(TAG, "mqtt_init OK");
//                 }
//                 // reset flags
//                 ble_started  = false;
//                 wifi_was_up   = true;
//             }
//         }

//         // đợi 60s rồi lặp lại
//         vTaskDelay(pdMS_TO_TICKS(60000));
//     }
// }


static void on_ble_conn(uint16_t h) {
  conn_handle = h;   // gán cho biến riêng của bạn
}