#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// ==========================================
// THÔNG TIN MẠNG & BƯU ĐIỆN
// ==========================================
const char* ssid = "Hao";
const char* password = "abc12345";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Chân cắm LoRa trên ESP32
#define ss 5
#define rst 14
#define dio0 2

// ==========================================
// KẾT NỐI WI-FI VÀ MQTT
// ==========================================
void setup_wifi() {
  delay(10);
  Serial.println("\nDang ket noi Wi-Fi: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi da ket noi! IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Dang ket noi MQTT... ");
    String clientId = "Gateway_ESP32_" + String(random(0, 1000));
    if (client.connect(clientId.c_str())) {
      Serial.println("Thanh cong!");
    } else {
      Serial.print("Loi, ma: ");
      Serial.print(client.state());
      Serial.println(" Thu lai sau 5 giay");
      delay(5000);
    }
  }
}

// ==========================================
// KHỞI ĐỘNG HỆ THỐNG
// ==========================================
void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Loi: Khong tim thay LoRa tren ESP32!");
    while (1);
  }
  Serial.println("LoRa Gateway san sang nhan tin hieu!");
}

// ==========================================
// VÒNG LẶP CHÍNH: NGHE LORA & ĐẨY LÊN WEB
// ==========================================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Kiểm tra xem có gói tin LoRa nào bay tới không
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.println("Nhan duoc LoRa: " + incoming);

    // Giải mã JSON để lấy cái Tên (ID) của Node
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, incoming);
    
    if (!error) {
      String nodeId = doc["id"].as<String>();
      
      // Nếu gói tin hợp lệ và có chứa Tên ID
      if (nodeId != "null" && nodeId != "") {
        // Tự động tạo đường dẫn Topic theo tên Node
        String topic = "hao_chinh_phuc_iot_2026/" + nodeId + "/du_lieu";
        
        // Đẩy toàn bộ gói JSON nguyên xi lên Web
        client.publish(topic.c_str(), incoming.c_str());
        Serial.println(" -> Da day len MQTT topic: " + topic);
      }
    } else {
      Serial.println("Loi: Goi tin LoRa khong phai chuan JSON.");
    }
  }
}