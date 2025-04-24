#ifndef MQTT_CL_H
#define MQTT_CL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "mqtt_client.h"

// Callback khi có message đến
// topic, payload (không null-terminated), độ dài payload
typedef void (*mqtt_msg_cb_t)(const char *topic, const char *payload, int len);

// Khởi tạo và kết nối MQTT. 
// uri ví dụ "mqtt://broker.hivemq.com", client_id tự chọn.
// msg_cb là callback khi có message mới.
esp_err_t mqtt_init(const char *uri, const char *client_id, mqtt_msg_cb_t msg_cb);

// Gửi message. retain nếu muốn giữ lại trên broker.
// Trả về ESP_OK nếu gọi thành công.
esp_err_t mqtt_publish_message(const char *topic, const char *payload, int len, int qos, bool retain);

// Đăng ký subscribe topic với QoS (0 hoặc 1).
esp_err_t mqtt_subscribe_topic(const char *topic, int qos);

// Ngắt kết nối và giải phóng tài nguyên.
esp_err_t mqtt_deinit(void);




#ifdef __cplusplus
}
#endif
#endif // MQTT_CL_H