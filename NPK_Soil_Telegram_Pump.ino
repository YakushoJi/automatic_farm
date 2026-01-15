#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ==========================================
// 1. WiFi and Telegram Setup
// ==========================================
const char* ssid = "NPK_FARM";           // WiFi Name
const char* password = "NPK_FARM";       // WiFi Password

// *** Don't forget to update the Token if you create a new bot ***
const char* botToken = "8314340056:AAEbYV78E13oTwjTq19H3PJdDqtldtOa6Nk";
const char* chatID = "-5058470466";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

unsigned long lastTimeBotRan = 0;
const long botRequestDelay = 60000;      // Send data to phone every 1 minute (60000 ms)

// Variables for WiFi connection check
unsigned long previousWifiCheckMillis = 0;
const long wifiCheckInterval = 20000;    // Check WiFi status every 20 seconds

// ==========================================
// 2. OLED Display Setup (I2C)
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// 3. NPK Sensor Setup (RS485)
// ==========================================
// Using Serial2: RX=26, TX=27
#define RE 5  // Receiver Enable
#define DE 4  // Driver Enable

// Hex commands to request N, P, K values (Modbus RTU)
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[]  = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[]  = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
byte values[11]; // Array to store response bytes

// ==========================================
// 4. Soil Moisture & Relay Setup
// ==========================================
const int PIN_SOIL = 32;  // Analog pin for soil moisture
const int PIN_RELAY = 15; // Relay pin (Changed from 26 to 15 to avoid RS485 conflict)

// Calibration values (MUST be adjusted based on real measurements)
int rawDry = 3700; // Raw value when dry (in air)
int rawWet = 1600; // Raw value when wet (in water)

// Pump control thresholds (%)
int TH_LOW = 35;   // If moisture < 35% -> Turn pump ON
int TH_HIGH = 60;  // If moisture > 60% -> Turn pump OFF

bool relayActiveLow = true; // true = LOW trigger (Common for relays)
bool pumpOn = false;        // Stores current pump status

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

  // --- Setup RS485 (Serial2) ---
  Serial2.begin(4800, SERIAL_8N1, 26, 27); // Baud 4800, RX=26, TX=27
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  // --- Setup Relay & Soil Sensor ---
  pinMode(PIN_RELAY, OUTPUT);
  setPump(false); // Ensure pump is OFF at startup

  // --- Setup OLED ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Stop here if display fails
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
  client.setInsecure(); // Allow connection without certificate
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Auto Reconnect WiFi System
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousWifiCheckMillis >= wifiCheckInterval)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousWifiCheckMillis = currentMillis;
  }

  // ==========================================
  // Section 1: Read Moisture & Control Pump
  // ==========================================
  // Read average value to reduce noise
  const int N = 10;
  long sum = 0;
  for (int i = 0; i < N; i++) {
    sum += analogRead(PIN_SOIL);
    delay(5);
  }
  int rawSoil = sum / N;
  int moisturePct = moisturePercentFromRaw(rawSoil); // Convert to %

  // Control Pump (Hysteresis Logic)
  if (moisturePct <= TH_LOW) {
    setPump(true);  // Dry -> Pump ON
  } else if (moisturePct >= TH_HIGH) {
    setPump(false); // Wet -> Pump OFF
  }
  // Note: Between 36-59%, the pump keeps its previous state

  // ==========================================
  // Section 2: Read NPK Values
  // ==========================================
  int valN = nitrogen();     delay(100);
  int valP = phosphorous();  delay(100);
  int valK = potassium();    delay(100);

  // Debug via Serial Monitor
  Serial.printf("Moist: %d%% (%d) | Pump: %s | N: %d P: %d K: %d\n", 
                moisturePct, rawSoil, pumpOn ? "ON":"OFF", valN, valP, valK);

  // ==========================================
  // Section 3: OLED Display
  // ==========================================
  display.clearDisplay();
  
  // Line 1: WiFi & Pump Status
  display.setTextSize(1);
  display.setCursor(0, 0);
  if(WiFi.status() == WL_CONNECTED) display.print("WiFi:OK ");
  else display.print("No WiFi ");
  
  display.print("| Pump:");
  if(pumpOn) display.print("ON"); else display.print("OFF");

  // Line 2: Soil Moisture
  display.setCursor(0, 15);
  display.print("Moist: "); display.print(moisturePct); display.print("%");

  // Lines 3-5: NPK Values
  display.setCursor(0, 30);
  display.print("N: "); display.print(valN); display.print(" mg");
  
  display.setCursor(0, 42);
  display.print("P: "); display.print(valP); display.print(" mg");
  
  display.setCursor(0, 54);
  display.print("K: "); display.print(valK); display.print(" mg");

  display.display();

  // ==========================================
  // Section 4: Send Telegram Report
  // ==========================================
  if (currentMillis > lastTimeBotRan + botRequestDelay) {
    if(WiFi.status() == WL_CONNECTED){
      String message = "Farm Status Report:\n";
      message += "💧 Soil Moisture: " + String(moisturePct) + " %\n";
      message += "🚜 Water Pump: " + String(pumpOn ? "Working (ON)" : "Stopped (OFF)") + "\n";
      message += "------------------\n";
      message += "🌱 N: " + String(valN) + " mg/kg\n";
      message += "🌱 P: " + String(valP) + " mg/kg\n";
      message += "🌱 K: " + String(valK) + " mg/kg";
      
      if(bot.sendMessage(chatID, message, "")){
        Serial.println("Telegram Sent Successfully");
      } else {
        Serial.println("Telegram Failed to Send");
      }
    }
    lastTimeBotRan = currentMillis;
  }
  
  delay(1000); // Main loop delay
}

// ==========================================
// Helper Functions
// ==========================================

// Convert raw analog value to percentage (0-100%)
int moisturePercentFromRaw(int raw) {
  int pct = map(raw, rawDry, rawWet, 0, 100);
  return constrain(pct, 0, 100); // Ensure value stays within 0-100
}

// Control the Relay
void setPump(bool on) {
  pumpOn = on;
  // Check active low/high logic
  int level = on ? (relayActiveLow ? LOW : HIGH) : (relayActiveLow ? HIGH : LOW);
  digitalWrite(PIN_RELAY, level);
}

// Read Nitrogen Value
int nitrogen(){
  while(Serial2.available()) Serial2.read(); // Clear buffer
  digitalWrite(DE,HIGH); digitalWrite(RE,HIGH); delay(1); // Enable TX
  if(Serial2.write(nitro,sizeof(nitro))==8){
    Serial2.flush(); // Wait for TX to complete
    digitalWrite(DE,LOW); digitalWrite(RE,LOW); // Enable RX
    unsigned long startTime = millis();
    while(Serial2.available() < 7 && millis() - startTime < 200); // Wait for response
    if(Serial2.available() >= 7){
       for(byte i=0;i<7;i++){ values[i] = Serial2.read(); }
       return (values[3] << 8) | values[4]; // Combine High & Low byte
    }
  }
  return 0; // Return 0 if failed
}

// Read Phosphorous Value
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

// Read Potassium Value
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
