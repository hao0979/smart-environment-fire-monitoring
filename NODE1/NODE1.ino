#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define DHTPIN 2              
#define DHTTYPE DHT22         
#define CHAN_KHOI A0          
#define CHAN_LUA 4            

#define ss 10
#define rst 9
#define dio0 5                

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

unsigned long thoiGianTruoc = 0;
const long chuKyGui = 3000;   
int trangThaiLuaCu = 0;

void setup() {
  Serial.begin(115200);
  pinMode(CHAN_LUA, INPUT);
  dht.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Khoi dong...");

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    lcd.clear(); lcd.print("Loi LoRa!");
    while (1);
  }
  delay(1000); lcd.clear();
}

void guiVaHienThi(float t, float h, int g, int f) {
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t, 1);
  lcd.print("C H:"); lcd.print(h, 0); lcd.print("%  ");

  lcd.setCursor(0, 1);
  if (f == 1) {
    lcd.print("!!! CO LUA !!!  ");
  } else {
    lcd.print("G:"); lcd.print(g); lcd.print("  An toan   ");
  }

  // --- BÍ MẬT NẰM Ở ĐÂY ---
  StaticJsonDocument<200> doc;
  
  // Gắn BẢNG TÊN cho Node. Trang Web sẽ lấy tên này để tạo thẻ.
  doc["id"] = "A"; 
  
  if (!isnan(t) && !isnan(h)) {
    doc["T"] = t; 
    doc["H"] = h;
  }
  doc["G"] = g; 
  doc["F"] = f;

  LoRa.beginPacket();
  serializeJson(doc, LoRa);
  LoRa.endPacket();
}

void loop() {
  int docLua = digitalRead(CHAN_LUA);
  int luaHienTai = (docLua == LOW) ? 1 : 0; 

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int g = analogRead(CHAN_KHOI);

  if (luaHienTai == 1 && trangThaiLuaCu == 0) {
    guiVaHienThi(t, h, g, luaHienTai);
    trangThaiLuaCu = 1;
    thoiGianTruoc = millis();
  } 
  else if (luaHienTai == 0 && trangThaiLuaCu == 1) {
    trangThaiLuaCu = 0;
  }

  if (millis() - thoiGianTruoc >= chuKyGui) {
    guiVaHienThi(t, h, g, luaHienTai);
    thoiGianTruoc = millis();
  }
  delay(100); 
}