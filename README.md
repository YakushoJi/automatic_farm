# 🌱 ESP32 Smart Farm IoT System
> **Monitor NPK, Soil Moisture, and Auto-Watering with Telegram Alerts**

![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![Platform](https://img.shields.io/badge/Platform-ESP32-orange.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)

## 📖 Overview
This project is a complete **Smart Agriculture solution** powered by an **ESP32**. It allows you to monitor soil health (Nitrogen, Phosphorus, Potassium) and moisture levels in real-time. The system automatically controls a water pump based on moisture thresholds and sends detailed status reports directly to your smartphone via Telegram.

---

## ✨ Key Features

| Feature | Description |
| :--- | :--- |
| **🧪 NPK Monitoring** | Reads soil nutrients (N, P, K) using RS485 Modbus industrial sensors. |
| **💧 Smart Watering** | Auto-activates pump when moisture is **< 35%** and stops at **> 60%**. |
| **📱 Telegram Bot** | Sends a report every **60 seconds** with current sensor readings & pump status. |
| **📟 OLED Dashboard** | Displays real-time data, WiFi status, and active components on a 0.96" screen. |
| **🔄 Auto-Reconnect** | Built-in redundancy to reconnect to WiFi automatically if signal is lost. |

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 Development Board
* **Sensors:**
    * NPK Soil Sensor (RS485 Modbus)
    * Capacitive Soil Moisture Sensor (Analog)
* **Modules:**
    * MAX485 / RS485 to TTL Adapter
    * Relay Module (5V/12V)
    * OLED Display 0.96" (I2C)
* **Power:** 5V-12V Power Supply (NPK sensors typically need 12V).

---

## 🔌 Wiring Diagram

Connect your components to the ESP32 as follows:

| Component | Pin Name | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **OLED Display** | SDA | `21` | I2C Data |
| | SCL | `22` | I2C Clock |
| **RS485 Module** | RO (RX) | `26` | Serial2 RX |
| | DI (TX) | `27` | Serial2 TX |
| | DE | `4` | Write Enable |
| | RE | `5` | Read Enable |
| **Sensors** | Soil Moisture | `32` | Analog Input |
| **Actuators** | Relay (Pump) | `15` | Digital Output |

> **⚠️ Important:** Ensure the Ground (GND) of the external power supply is connected to the ESP32 GND.

---

## ⚙️ Configuration

Open `main.cpp` and update the following credentials before uploading:

### 1. WiFi & Telegram
```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* chatID   = "YOUR_TELEGRAM_CHAT_ID";
