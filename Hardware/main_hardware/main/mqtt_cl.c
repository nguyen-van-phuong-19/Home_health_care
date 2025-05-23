#include "mqtt_cl.h"
#include "esp_err.h"
#include "sensor_data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *TAG = "MQTT_CL";
static bool parse_sleep_duration(const char *data, int data_len, float *out_val);
static esp_mqtt_client_handle_t client = NULL;
static mqtt_msg_cb_t    user_msg_cb = NULL;
// EventGroup để signal đã kết nối
EventGroupHandle_t mqtt_event_group;
const int MQTT_CONNECTED_BIT = BIT0;
const int MQTT_CLEANED_BIT   = BIT1;

static esp_err_t _mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGI(TAG, "MQTT connected");
            mqtt_subscribe_topic(MQTT_TOPIC_HEART_RATE, 0);
            mqtt_subscribe_topic(MQTT_TOPIC_SPO2, 0);
            mqtt_subscribe_topic(MQTT_TOPIC_ACCELEROMETER, 0);
            mqtt_subscribe_topic(MQTT_TOPIC_CALORIES, 0);
            mqtt_subscribe_topic(MQTT_TOPIC_SLEEP, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGW(TAG, "MQTT disconnected");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Incoming[%.*s]: %.*s",
                     event->topic_len, event->topic,
                     event->data_len,  event->data);
            if (event->topic_len == strlen(MQTT_TOPIC_SLEEP)
                && strncmp(event->topic, MQTT_TOPIC_SLEEP, event->topic_len) == 0) {
                float sleep_hours;
                if (parse_sleep_duration(event->data, event->data_len, &sleep_hours)) {
                    ESP_LOGI(TAG, "Parsed sleep_duration = %.2f hours", sleep_hours);
                    sensor_data_set_sleep_hours(sleep_hours);
                    // TODO: lưu hoặc xử lý sleep_hours tại đây
                } else {
                    ESP_LOGW(TAG, "Failed to parse sleep_duration");
                }
            }
            if (user_msg_cb) {
                user_msg_cb(event->topic, event->data, event->data_len);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void _mqtt_event_handler(void *handler_args,
                                esp_event_base_t base,
                                int32_t id,
                                void *event_data) {
    _mqtt_event_handler_cb(event_data);
}

esp_err_t mqtt_init(const char *uri, const char *client_id, mqtt_msg_cb_t msg_cb) {
    if (client) {
        ESP_LOGW(TAG, "MQTT already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    user_msg_cb = msg_cb;

    mqtt_event_group = xEventGroupCreate();
    
    esp_mqtt_client_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.broker.address.uri    = uri;
    cfg.credentials.client_id = client_id;

    client = esp_mqtt_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                   _mqtt_event_handler, NULL);
    xEventGroupClearBits(mqtt_event_group,
                    MQTT_CONNECTED_BIT | MQTT_CLEANED_BIT);
    return esp_mqtt_client_start(client);
}

esp_err_t mqtt_publish_message(const char *topic,
                               const char *payload,
                               int len,
                               int qos,
                               bool retain) {
    if (!client) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_publish(client, topic, payload, len, qos, retain);
    return (msg_id < 0) ? ESP_FAIL : ESP_OK;
}

esp_err_t mqtt_subscribe_topic(const char *topic, int qos) {
    if (!client) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    ESP_LOGE(TAG, "msg_id: %d", msg_id);
    return (msg_id < 0) ? ESP_FAIL : ESP_OK;
}

esp_err_t mqtt_deinit(void) {
    if (!client) return ESP_ERR_INVALID_STATE;
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    client = NULL;
    user_msg_cb = NULL;
    xEventGroupSetBits(mqtt_event_group, MQTT_CLEANED_BIT);
    return ESP_OK;
}

esp_err_t mqtt_publish_heart_rate(const char* user_id,
                                   int bpm,
                                   float weight_kg,
                                   int age,
                                   int epoch_minutes) {
    char payload[256];
    int len = snprintf(payload, sizeof(payload),
        "{\"user_id\":\"%s\",\"bpm\":%d,\"weight_kg\":%.1f,\"age\":%d,\"epoch_minutes\":%d}",
        user_id, bpm, weight_kg, age, epoch_minutes);
    return mqtt_publish_message(MQTT_TOPIC_HEART_RATE, payload, len, 1, false);
}

esp_err_t mqtt_publish_spo2(const char* user_id,
                             float percentage) {
    char payload[128];
    int len = snprintf(payload, sizeof(payload),
        "{\"user_id\":\"%s\",\"percentage\":%.1f}",
        user_id, percentage);
    return mqtt_publish_message(MQTT_TOPIC_SPO2, payload, len, 1, false);
}

esp_err_t mqtt_publish_accelerometer(const char* user_id,
                                      float total_vector,
                                      float weight_kg,
                                      int epoch_minutes) {
    char payload[256];
    int len = snprintf(payload, sizeof(payload),
        "{\"user_id\":\"%s\",\"total_vector\":%.2f,\"weight_kg\":%.1f,\"epoch_minutes\":%d}",
        user_id, total_vector, weight_kg, epoch_minutes);
    return mqtt_publish_message(MQTT_TOPIC_ACCELEROMETER, payload, len, 1, false);
}

esp_err_t mqtt_publish_gps(const char* user_id,
                            double latitude,
                            double longitude,
                            double altitude) {
    char payload[256];
    int len = snprintf(payload, sizeof(payload),
        "{\"user_id\":\"%s\",\"latitude\":%.6f,\"longitude\":%.6f,\"altitude\":%.2f}",
        user_id, latitude, longitude, altitude);
    return mqtt_publish_message(MQTT_TOPIC_GPS, payload, len, 1, false);
}


void mqtt_publish_calories(void) {
    float cal_hr = 0.0f;
    float cal_motion = 0.0f;
    float cal_total = 0.0f;

    // Lấy dữ liệu calories
    if (sensor_data_get_calories_hr(&cal_hr) != ESP_OK ||
        sensor_data_get_calories_motion(&cal_motion) != ESP_OK ||
        sensor_data_get_daily_calories(&cal_total) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to retrieve calories data");
        return;
    }

    // Tạo payload JSON
    char payload[128];
    int len = snprintf(payload, sizeof(payload),
                       "{\"cal_hr\":%.2f,\"cal_motion\":%.2f,\"cal_total\":%.2f}",
                       cal_hr, cal_motion, cal_total);
    if (len < 0 || len >= sizeof(payload)) {
        ESP_LOGE(TAG, "Payload formatting error");
        return;
    }
    mqtt_publish_message(MQTT_TOPIC_CALORIES, payload, len, 1, false);
}


static bool parse_sleep_duration(const char *data, int data_len, float *out_val)
{
    // Copy vào buffer null-terminated
    char buf[64];
    int  len = data_len < (int)sizeof(buf)-1 ? data_len : (int)sizeof(buf)-1;
    memcpy(buf, data, len);
    buf[len] = '\0';

    // 1) Thử parse JSON có key "sleep_duration"
    char *p = strstr(buf, "\"sleep_duration\"");
    if (p) {
        p = strchr(p, ':');
        if (p && (sscanf(p+1, " %f", out_val) == 1)) {
            return true;
        }
        return false;
    }

    // 2) Nếu không có JSON, thử parse như một số đơn thuần
    //    Tìm số đầu tiên trong chuỗi
    if (sscanf(buf, " %f", out_val) == 1) {
        return true;
    }

    // Không parse được
    return false;
}

/*
 * Example usage:
 *   mqtt_init("mqtt://broker.hivemq.com", "device123", NULL);
 *   // Then in your sensor task:
 *   mqtt_publish_heart_rate("user123", 72, 70.0f, 30, 1);
 *   mqtt_publish_spo2("user123", 98.5f);
 *   mqtt_publish_accelerometer("user123", 9.81f, 70.0f, 1);
 *   mqtt_publish_gps("user123", 13.7563, 100.5018, 10.0);
 */
