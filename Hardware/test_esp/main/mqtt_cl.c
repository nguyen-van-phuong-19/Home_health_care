// mqtt_cl.c
#include "mqtt_cl.h"

static const char *TAG = "MQTT_CL";
static esp_mqtt_client_handle_t client = NULL;
static mqtt_msg_cb_t    user_msg_cb = NULL;

// Dùng EventGroup để signal đã kết nối
static EventGroupHandle_t mqtt_event_group;
const int MQTT_CONNECTED_BIT = BIT0;


static esp_err_t _mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGW(TAG, "MQTT disconnected");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Subscribed to topic, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Unsubscribed, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Published, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Incoming[%.*s]: %.*s",
                     event->topic_len, event->topic,
                     event->data_len,  event->data);
            if (user_msg_cb) {
                user_msg_cb(event->topic, event->data, event->data_len);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data){
    _mqtt_event_handler_cb(event_data);
}

esp_err_t mqtt_init(const char *uri, const char *client_id, mqtt_msg_cb_t msg_cb) {
    if (client) {
        ESP_LOGW(TAG, "MQTT already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    user_msg_cb = msg_cb;

    // Khởi tạo struct và zero toàn bộ
    esp_mqtt_client_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    // Gán broker URI và client_id vào đúng chỗ
    cfg.broker.address.uri         = uri;
    cfg.credentials.client_id      = client_id;
    // nếu cần username/password thì:
    // cfg.credentials.username   = "your_user";
    // cfg.credentials.authentication.password = "your_pass";

    client = esp_mqtt_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, _mqtt_event_handler, NULL);
    return esp_mqtt_client_start(client);
}

esp_err_t mqtt_publish_message(const char *topic, const char *payload, int len, int qos, bool retain) {
    if (!client) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_publish(client, topic, payload, len, qos, retain);
    return (msg_id < 0) ? ESP_FAIL : ESP_OK;
}

esp_err_t mqtt_subscribe_topic(const char *topic, int qos) {
    if (!client) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    return (msg_id < 0) ? ESP_FAIL : ESP_OK;
}

esp_err_t mqtt_deinit(void) {
    if (!client) return ESP_ERR_INVALID_STATE;
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    client = NULL;
    user_msg_cb = NULL;
    return ESP_OK;
}
