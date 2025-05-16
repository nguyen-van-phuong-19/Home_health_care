#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "main.h"
#include "l80r_process.h"
#include "mqtt_cl.h"
#include "sensor_data.h"
#include <stdbool.h>
// #include "ble_store.h"



#define LIS2DH12_TASK_STACK_SIZE    (8*1024)
#define MAX30102_TASK_STACK_SIZE    4096
#define LIS2DH12_TASK_PRIORITY      5
#define MAX30102_TASK_PRIORITY      5
#define WIFI_WATCHDOG_TASK_PRIORITY 12
#define TRANSMIT_TASK_PRIORITY      3

static const char *TAG = "APP_MAIN";

// Shared I2C mutex to protect bus on SDA/SCL
static SemaphoreHandle_t i2c_mutex = NULL;

static StaticTask_t lis2dh12_tcb;
static StackType_t  lis2dh12_stack[LIS2DH12_TASK_STACK_SIZE];
static StaticTask_t max30102_tcb;
static StackType_t  max30102_stack[MAX30102_TASK_STACK_SIZE];
// cấu hình stack/TCB để tạo task tĩnh
#define WIFI_WD_STACK_SIZE 4096
static StaticTask_t   wifiWdTCB;
static StackType_t    wifiWdStack[WIFI_WD_STACK_SIZE];


static void lis2dh12_task(void *arg);
static void max30102_task(void *arg);
static void wifi_watchdog_task(void *arg);
static void transmit_task(void *pv);

static uint32_t red_block[BLOCK_SIZE];
static uint32_t ir_block[BLOCK_SIZE];
static int block_index = 0;
static int convolved_index = 0;
static float convolved_signal[CONVOLVED_SIZE];
static float vector_sum = 0.0f;

void app_main(void)
{
    wifi_init_sta("102", "11111111");

    // 2) Khởi tạo LIS2DH12TR
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_ERROR_CHECK(lis2dh12_init());
    ESP_ERROR_CHECK(max30102_init());
    ESP_LOGI("MAIN", "I2C initialized");

    l80r_init();
    // // Create I2C mutex (with priority inheritance)
    i2c_mutex = xSemaphoreCreateMutex();
    configASSERT(i2c_mutex);

    ESP_ERROR_CHECK(sensor_data_init());

    BaseType_t r;

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

    xTaskCreateStatic(
        wifi_watchdog_task,         // entry fn
        "wifi_watchdog",            // name
        WIFI_WD_STACK_SIZE,         // stack depth (words)
        NULL,                       // param
        WIFI_WATCHDOG_TASK_PRIORITY,       // priority
        wifiWdStack,                // stack buffer
        &wifiWdTCB                  // TCB buffer
    );

    r = xTaskCreate(
        transmit_task,
        "transmit",
        4*1024,
        NULL,
        TRANSMIT_TASK_PRIORITY,  // ưu tiên cao hơn các task thu thập dữ liệu
        NULL
    );
    if (r != pdPASS) {
        ESP_LOGE(TAG, "Failed to create transmit_task");
    }

    xTaskCreate(l80r_task, "l80r_task", 4096, NULL, 5, NULL);

}


static void lis2dh12_task(void *arg)
{
    lis2dh12_vector_t acc;
    lis2dh12_vector_t acc_last;
    for(;;) {
        if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (lis2dh12_get_vector(&acc) == ESP_OK) {
                vector_sum = vector_sum + sqrtf((acc.x - acc_last.x) * (acc.x - acc_last.x) + (acc.y - acc_last.y) * (acc.y - acc_last.y) + (acc.z - acc_last.z) * (acc.z - acc_last.z));
                convolved_index++;
                acc_last = acc;
                // printf("Accel [m/s^2]: X=%7.3f  Y=%7.3f  Z=%7.3f\n",
                //     acc.x, acc.y, acc.z);

                if(convolved_index >= 60 * 100){
                    ESP_LOGI(TAG, "total vector: %.2f",
                                vector_sum);
                    sensor_data_set_motion(vector_sum);
                    vector_sum = 0.0f;
                    convolved_index = 0;
                }
            } else {
                ESP_LOGE(TAG, "LIS2DH12TR read failed");
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
                    sensor_data_set_hr(hr);
                    sensor_data_set_spo2(spo2_avg);
                    // Reset index để block mới
                    block_index = 0;
                    for(int i = 0; i < BLOCK_SIZE; i++) {
                        red_block[i] = 0;
                        ir_block[i]  = 0;
                    }

                    xSemaphoreGive(i2c_mutex);
                    vTaskDelay(pdMS_TO_TICKS(50000)); // Chờ 10 giây trước khi thu thập block tiếp theo
                    continue;
                }
            } else {
                ESP_LOGE(TAG, "Error reading sample");
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));  // 1 Hz
    }

}

static void wifi_watchdog_task(void *arg)
{
    EventGroupHandle_t eg = wifi_event_group;
    bool wifi_was_up   = true;
    bool ble_started   = false;
    bool mqtt_up       = false;

    for (;;) {
        EventBits_t bits = xEventGroupGetBits(eg);

        if ((bits & WIFI_CONNECTED_BIT) == 0) {
            // —— Wi-Fi DOWN ——
            if (wifi_was_up) {
                // lần đầu phát hiện rớt, dọn dẹp MQTT + khởi BLE
                ESP_LOGW(TAG, "WiFi lost: deinit MQTT, init BLE");
                if (mqtt_deinit() == ESP_OK) {
                    ESP_LOGI(TAG, "mqtt_deinit OK");
                }
                if (ble_app_init() == ESP_OK) {
                    ESP_LOGI(TAG, "ble_init OK");
                    ble_started = true;
                }
                wifi_was_up   = false;
                mqtt_up       = false;
            }

            // thử reconnect Wi-Fi
            ESP_LOGI(TAG, "Watchdog: esp_wifi_connect()");
            esp_err_t err = esp_wifi_connect();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_wifi_connect() failed: %d", err);
            }

        } else {
            if (mqtt_up) {
              EventBits_t mqtt_bits = xEventGroupGetBits(mqtt_event_group);
              if (mqtt_bits & MQTT_CONNECTED_BIT) {
                ESP_LOGW(TAG, "WiFi OK but MQTT not connected");
                mqtt_up       = false;
              }
            }
            // —— Wi-Fi UP ——
            if (!wifi_was_up) {
                // lần đầu phát hiện lên lại, dọn BLE + khởi MQTT
                ESP_LOGI(TAG, "WiFi reconnected: deinit BLE, init MQTT");
                if (ble_started && ble_deinit() == ESP_OK) {
                    ESP_LOGI(TAG, "ble_deinit OK");
                }
                // Giả sử bạn muốn tự động tái-khởi MQTT:
                if (mqtt_init("mqtt://broker.emqx.io:1883", "client_id", NULL) == ESP_OK) {
                    ESP_LOGI(TAG, "mqtt_init OK");
                }
                // reset flags
                ble_started  = false;
                wifi_was_up   = true;
                mqtt_up      = true;
            } else if(!mqtt_up){
                if (mqtt_init("mqtt://broker.emqx.io:1883", "client_id", NULL) == ESP_OK) {
                    ESP_LOGI(TAG, "mqtt_init OK");
                }
                mqtt_up      = true;
            }
        }
        ESP_LOGI(TAG, "Check wifi!");
        // đợi 60s rồi lặp lại
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}




static void transmit_task(void *pv)
{
    uint8_t hr;
    uint8_t spo2;
    float   motion;
    const char *user_id = "2mrSt8vHRQd6kpPiHjuLobCrwK13";

    // Chọn chu kỳ gửi — ví dụ 1s
    const TickType_t delay_ticks = pdMS_TO_TICKS((1000 * 60) + 1000);

    while (1) {
        // 1) Lấy đồng thời cả 3 giá trị (atomic dưới mutex)
        if (sensor_data_get_all(&hr, &spo2, &motion) != ESP_OK) {
            ESP_LOGW(TAG, "Cannot read sensor data");
            vTaskDelay(delay_ticks);
            continue;
        }

        // 2) Kiểm tra Wi-Fi
        EventBits_t wifi_bits = xEventGroupGetBits(wifi_event_group);
        if (wifi_bits & WIFI_CONNECTED_BIT) {
            // 2a) Nếu Wi-Fi OK → kiểm tra MQTT
            EventBits_t mqtt_bits = xEventGroupGetBits(mqtt_event_group);
            if (mqtt_bits & MQTT_CONNECTED_BIT) {
                // Gửi qua MQTT
                // Nếu bạn đã có các hàm publish riêng, gọi trực tiếp
                // l80r_data_t current;
                // l80r_get_data(&current);
                esp_err_t err;
                err = mqtt_publish_heart_rate(user_id,hr,75,23, 1);
                if (err != ESP_OK) {
                    ESP_LOGW(TAG, "MQTT HR publish failed: %s", esp_err_to_name(err));
                }
                err = mqtt_publish_spo2(user_id,spo2);
                if (err != ESP_OK) {
                    ESP_LOGW(TAG, "MQTT SpO2 publish failed: %s", esp_err_to_name(err));
                }
                err = mqtt_publish_accelerometer(user_id, motion, 75, 1);
                if (err != ESP_OK) {
                    ESP_LOGW(TAG, "MQTT Motion publish failed: %s", esp_err_to_name(err));
                }
                // err = mqtt_publish_gps("user123", current.latitude, current.longitude, current.altitude);
                // if(err != ESP_OK){
                //     ESP_LOGW(TAG, "MQTT GPS publish failed: %s", esp_err_to_name(err));
                // }
            } else {
                ESP_LOGW(TAG, "WiFi OK but MQTT not connected");
                // Có thể trigger reconnect hoặc log, tuỳ bạn
            }
        } else {
            // 2b) Nếu không có Wi-Fi → gửi qua BLE advertising
            // Bạn cần convert 3 giá trị vào 1 mảng bytes
            // Ví dụ: [hr(1B), spo2(1B), motion(4B float little-endian)]
            uint8_t buf[6];
            buf[0] = hr;
            buf[1] = spo2;
            // memcopy float vào buf[2..5]
            memcpy(&buf[2], &motion, sizeof(motion));

            if (update_service_data(buf, sizeof(buf)) != ESP_OK) {
                ESP_LOGW(TAG, "BLE adv update failed");
            }
            // BLE advertising phải đã start sẵn ở init, bạn chỉ cần update payload
        }

        vTaskDelay(delay_ticks);
    }
}
