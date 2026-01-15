#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ==========================================
// 1. ตั้งค่า WiFi และ Telegram
// ==========================================
const char* ssid = "NPK_FARM";
const char* password = "NPK_FARM";
// *** อย่าลืมเปลี่ยน Token ใหม่เพื่อความปลอดภัย ***
const char* botToken = "8314340056:AAEbYV78E13oTwjTq19H3PJdDqtldtOa6Nk";
const char* chatID = "-5058470466";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

unsigned long lastTimeBotRan = 0;
const long botRequestDelay = 60000; // ส่งค่าเข้ามือถือทุก 1 นาที

// ตัวแปรสำหรับเช็ค WiFi
unsigned long previousWifiCheckMillis = 0;
const long wifiCheckInterval = 20000;

// ==========================================
// 2. ตั้งค่าจอ OLED
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// 3. ตั้งค่า NPK (RS485)
// ==========================================
// RX=26, TX=27 (กำหนดใน Serial2.begin)
#define RE 5
#define DE 4

const byte nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
byte values[11];

// ==========================================
// 4. ตั้งค่า Soil Moisture & Relay
// ==========================================
const int PIN_SOIL = 32;  // ขาอ่านความชื้น (ADC)
const int PIN_RELAY = 15; // *** แก้จาก 26 เป็น 15 เพราะ 26 ชนกับ RS485 ***

// ค่าคาลิเบรต (ต้องแก้ตามค่าจริงที่คุณวัดได้)
int rawDry = 3700; // ค่าตอนแห้ง
int rawWet = 1600; // ค่าตอนเปียก

// เกณฑ์ตัดสินใจ (%)
int TH_LOW = 35;   // ต่ำกว่า 35% เปิดน้ำ
int TH_HIGH = 60;  // สูงกว่า 60% ปิดน้ำ

bool relayActiveLow = true; // true = สั่ง LOW เพื่อเปิดปั๊ม
bool pumpOn = false;        // เก็บสถานะปั๊มปัจจุบัน

// ==========================================
// Prototype Functions
// ==========================================
int nitrogen();
int phosphorous();
int potassium();
int moisturePercentFromRaw(int raw);
void setPump(bool on);

void setup() {
  Serial.begin(9600);

  // --- Setup RS485 ---
  Serial2.begin(4800, SERIAL_8N1, 26, 27);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  // --- Setup Relay & Soil ---
  pinMode(PIN_RELAY, OUTPUT);
  setPump(false); // เริ่มต้นปิดปั๊ม

  // --- Setup OLED ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Starting Farm OS...");
  display.display();

  // --- Setup WiFi ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setInsecure();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. ระบบ Reconnect WiFi
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousWifiCheckMillis >= wifiCheckInterval)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousWifiCheckMillis = currentMillis;
  }

  // ==========================================
  // ส่วนที่ 1: อ่านค่าความชื้นและคุมปั๊ม
  // ==========================================
  // อ่านค่าเฉลี่ย
  const int N = 10;
  long sum = 0;
  for (int i = 0; i < N; i++) {
    sum += analogRead(PIN_SOIL);
    delay(5);
  }
  int rawSoil = sum / N;
  int moisturePct = moisturePercentFromRaw(rawSoil);

  // ควบคุมปั๊ม (Logic Hysteresis)
  if (moisturePct <= TH_LOW) {
    setPump(true);  // แห้ง -> เปิดน้ำ
  } else if (moisturePct >= TH_HIGH) {
    setPump(false); // ชื้น -> ปิดน้ำ
  }
  // ถ้าอยู่ตรงกลาง (36-59%) ให้คงสถานะเดิมไว้

  // ==========================================
  // ส่วนที่ 2: อ่านค่า NPK
  // ==========================================
  int valN = nitrogen();     delay(100);
  int valP = phosphorous();  delay(100);
  int valK = potassium();    delay(100);

  // Debug ลง Serial Monitor
  Serial.printf("Moist: %d%% (%d) | Pump: %s | N: %d P: %d K: %d\n", 
                moisturePct, rawSoil, pumpOn ? "ON":"OFF", valN, valP, valK);

  // ==========================================
  // ส่วนที่ 3: แสดงผลจอ OLED
  // ==========================================
  display.clearDisplay();
  
  // บรรทัดบน: WiFi + Pump Status
  display.setTextSize(1);
  display.setCursor(0, 0);
  if(WiFi.status() == WL_CONNECTED) display.print("WiFi:OK ");
  else display.print("No WiFi ");
  
  display.print("| Pump:");
  if(pumpOn) display.print("ON"); else display.print("OFF");

  // แสดงค่าความชื้น (ตัวใหญ่หน่อย)
  display.setCursor(0, 15);
  display.print("Moist: "); display.print(moisturePct); display.print("%");

  // แสดงค่า NPK
  display.setCursor(0, 30);
  display.print("N: "); display.print(valN); display.print(" mg");
  
  display.setCursor(0, 42);
  display.print("P: "); display.print(valP); display.print(" mg");
  
  display.setCursor(0, 54);
  display.print("K: "); display.print(valK); display.print(" mg");

  display.display();

  // ==========================================
  // ส่วนที่ 4: ส่ง Telegram
  // ==========================================
  if (currentMillis > lastTimeBotRan + botRequestDelay) {
    if(WiFi.status() == WL_CONNECTED){
      String message = "รายงานสถานะฟาร์ม:\n";
      message += "💧 ความชื้นดิน: " + String(moisturePct) + " %\n";
      message += "🚜 ปั๊มน้ำ: " + String(pumpOn ? "ทำงาน (ON)" : "หยุด (OFF)") + "\n";
      message += "------------------\n";
      message += "🌱 N: " + String(valN) + " mg/kg\n";
      message += "🌱 P: " + String(valP) + " mg/kg\n";
      message += "🌱 K: " + String(valK) + " mg/kg";
      
      if(bot.sendMessage(chatID, message, "")){
        Serial.println("Telegram Sent");
      } else {
        Serial.println("Telegram Send Failed");
      }
    }
    lastTimeBotRan = currentMillis;
  }
  
  delay(1000); // หน่วงเวลารอบใหญ่
}

// ==========================================
// Helper Functions
// ==========================================

int moisturePercentFromRaw(int raw) {
  int pct = map(raw, rawDry, rawWet, 0, 100);
  return constrain(pct, 0, 100);
}

void setPump(bool on) {
  pumpOn = on;
  int level = on ? (relayActiveLow ? LOW : HIGH) : (relayActiveLow ? HIGH : LOW);
  digitalWrite(PIN_RELAY, level);
}

int nitrogen(){
  while(Serial2.available()) Serial2.read();
  digitalWrite(DE,HIGH); digitalWrite(RE,HIGH); delay(1);
  if(Serial2.write(nitro,sizeof(nitro))==8){
    Serial2.flush();
    digitalWrite(DE,LOW); digitalWrite(RE,LOW);
    unsigned long startTime = millis();
    while(Serial2.available() < 7 && millis() - startTime < 200);
    if(Serial2.available() >= 7){
       for(byte i=0;i<7;i++){ values[i] = Serial2.read(); }
       return (values[3] << 8) | values[4];
    }
  }
  return 0;
}

int phosphorous(){
  while(Serial2.available()) Serial2.read();
  digitalWrite(DE,HIGH); digitalWrite(RE,HIGH); delay(1);
  if(Serial2.write(phos,sizeof(phos))==8){
    Serial2.flush();
    digitalWrite(DE,LOW); digitalWrite(RE,LOW);
    unsigned long startTime = millis();
    while(Serial2.available() < 7 && millis() - startTime < 200);
    if(Serial2.available() >= 7){
       for(byte i=0;i<7;i++){ values[i] = Serial2.read(); }
       return (values[3] << 8) | values[4];
    }
  }
  return 0;
}

int potassium(){
  while(Serial2.available()) Serial2.read();
  digitalWrite(DE,HIGH); digitalWrite(RE,HIGH); delay(1);
  if(Serial2.write(pota,sizeof(pota))==8){
    Serial2.flush();
    digitalWrite(DE,LOW); digitalWrite(RE,LOW);
    unsigned long startTime = millis();
    while(Serial2.available() < 7 && millis() - startTime < 200);
    if(Serial2.available() >= 7){
       for(byte i=0;i<7;i++){ values[i] = Serial2.read(); }
       return (values[3] << 8) | values[4];
    }
  }
  return 0;
}