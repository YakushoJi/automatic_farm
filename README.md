# 🌱 ESP32 Smart Farm IoT System
> **An all-in-one solution for soil nutrient monitoring (NPK), moisture control, and remote alerts via Telegram.**

![Platform](https://img.shields.io/badge/Platform-ESP32-orange.svg)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Arduino-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)

## 📖 Overview
This project transforms a standard ESP32 into a **Smart Agriculture Controller**. It reads industrial-grade **RS485 NPK sensors** and capacitive soil moisture sensors to make intelligent watering decisions.

Users can view real-time data on an **OLED dashboard** or receive detailed status reports directly on their smartphone via a **Telegram Bot** every minute.

---

## ✨ Key Features

| Feature | Description |
| :--- | :--- |
| **🧪 Soil Nutrient Analysis** | Reads Nitrogen (N), Phosphorus (P), and Potassium (K) levels via Modbus RTU. |
| **💧 Auto-Watering System** | Automates the water pump based on soil moisture (On < 35% / Off > 60%). |
| **📱 Telegram Integration** | Sends periodic reports (Moisture %, Pump Status, NPK mg/kg) to your chat. |
| **📟 OLED Display** | Local dashboard showing WiFi status, pump state, and sensor values. |
| **🔄 Auto-Reconnect** | Robust WiFi handling system to ensure 24/7 connectivity. |

---

## 🛠️ Hardware Requirements

| Component | Quantity | Description |
| :--- | :--- | :--- |
| **ESP32 Dev Board** | 1 | The main microcontroller (30 pins or 38 pins). |
| **NPK Sensor (RS485)** | 1 | Industrial soil sensor (Modbus protocol). |
| **Soil Moisture Sensor** | 1 | Capacitive type (corrosion resistant). |
| **RS485 to TTL Module** | 1 | MAX485 module for communicating with the NPK sensor. |
| **OLED Display** | 1 | 0.96 inch I2C SSD1306 (128x64 pixels). |
| **Relay Module** | 1 | 1-Channel Relay (5V/12V) for controlling the pump. |
| **Power Supply** | 1 | 12V DC Adapter (required for NPK sensor & pump). |

---

## 🔌 Wiring Diagram / Pinout

Connect the components to the ESP32 as defined below:

| Component | Pin Name | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **OLED Display** | SDA | `21` | I2C Data |
| | SCL | `22` | I2C Clock |
| | VCC | `3.3V` or `5V` | |
| **RS485 Module** | DI (TX) | `27` | Connected to Serial2 TX |
| | RO (RX) | `26` | Connected to Serial2 RX |
| | DE | `4` | Write Enable Control |
| | RE | `5` | Read Enable Control |
| **Soil Sensor** | Analog Out | `32` | Capacitive Sensor Input |
| **Relay** | Signal (IN) | `15` | Pump Control |

> **⚠️ Important:** The NPK Sensor usually requires **5V-30V** external power. Do not power it directly from the ESP32's 3.3V pin. Ensure all Ground (GND) connections are common.

---

## ⚙️ Configuration & Setup

### 1. Library Installation
Install the following libraries via Arduino IDE or PlatformIO:
* `Adafruit GFX Library`
* `Adafruit SSD1306`
* `UniversalTelegramBot` (by Brian Lough)
* `ArduinoJson`

### 2. Credentials Setup
Open `main.cpp` and edit the configuration section:

```cpp
// ==========================================
// WiFi & Telegram Settings
// ==========================================
const char* ssid     = "YOUR_WIFI_NAME";      // Enter your WiFi Name
const char* password = "YOUR_WIFI_PASSWORD";  // Enter your WiFi Password
const char* botToken = "YOUR_BOT_TOKEN";      // Get from BotFather
const char* chatID   = "YOUR_CHAT_ID";        // Get from IDBot

