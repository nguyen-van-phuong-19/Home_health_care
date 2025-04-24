#ifndef MQTT_CL_H
#define MQTT_CL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <stdio.h>      // for snprintf
#include <time.h>       // for time(), localtime_r(), strftime
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "mqtt_client.h"

// --- MQTT topic definitions (must match server subscription) ---
#define MQTT_TOPIC_HEART_RATE        "wearable/heart_rate"
#define MQTT_TOPIC_SPO2              "wearable/spo2"
#define MQTT_TOPIC_ACCELEROMETER     "wearable/accelerometer"
#define MQTT_TOPIC_GPS               "wearable/gps"

extern EventGroupHandle_t mqtt_event_group;
extern const int MQTT_CONNECTED_BIT;

// Callback khi có message đến
// topic, payload (không null-terminated), độ dài payload
typedef void (*mqtt_msg_cb_t)(const char *topic, const char *payload, int len);

// Core init/publish/subscribe APIs
esp_err_t mqtt_init(const char *uri, const char *client_id, mqtt_msg_cb_t msg_cb);
esp_err_t mqtt_publish_message(const char *topic, const char *payload, int len, int qos, bool retain);
esp_err_t mqtt_subscribe_topic(const char *topic, int qos);
esp_err_t mqtt_deinit(void);

// --- High-level publish helpers ---
/**
 * @brief Publish heart rate payload
 */
esp_err_t mqtt_publish_heart_rate(const char* user_id,
                                   int bpm,
                                   float weight_kg,
                                   int age,
                                   int epoch_minutes);

/**
 * @brief Publish SpO2 payload
 */
esp_err_t mqtt_publish_spo2(const char* user_id,
                             float percentage);

/**
 * @brief Publish accelerometer payload
 */
esp_err_t mqtt_publish_accelerometer(const char* user_id,
                                      float total_vector,
                                      float weight_kg,
                                      int epoch_minutes);

/**
 * @brief Publish GPS payload
 */
esp_err_t mqtt_publish_gps(const char* user_id,
                            double latitude,
                            double longitude,
                            double altitude);



#ifdef __cplusplus
}
#endif
#endif // MQTT_CL_H