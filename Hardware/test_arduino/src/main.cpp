
#include <WiFi.h>

const char* ssid     = "nguyen_phuong";
const char* password = "00000000";

// Thời gian chờ tối đa (số lần loop) trước khi coi là fail
const int maxAttempts = 20;

void setup() {
  // Khởi Serial để xem log
  Serial.begin(9600);
  delay(1000);
  
  Serial.printf("Connecting to %s\n", ssid);

  // Chuyển sang chế độ STA và bắt đầu connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Đợi kết nối với timeout
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  Serial.println();

  // Kiểm tra kết quả
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi.");
  }
}

void loop() {
  Serial.println("WiFi connected!");
  delay(10000);
}
