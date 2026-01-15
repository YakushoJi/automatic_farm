# NPK Soil Moisture Telegram Pump Controller

An ESP32-based automated farm monitoring system that reads soil moisture and NPK (Nitrogen, Phosphorous, Potassium) levels, controls a water pump, and sends real-time reports via Telegram.

## Features

- 🌱 **NPK Sensor Integration**: Reads nitrogen, phosphorus, and potassium levels via RS485 Modbus RTU protocol
- 💧 **Soil Moisture Monitoring**: Continuous soil moisture measurement with hysteresis-based pump control
- 🚜 **Automatic Pump Control**: Smart pump activation based on moisture thresholds
- 📱 **Telegram Notifications**: Sends farm status reports every minute
- 📊 **OLED Display**: Real-time data visualization on 128x64 OLED screen
- 🔄 **Auto WiFi Reconnection**: Automatic WiFi recovery system
- 📡 **RS485 Communication**: Modbus RTU protocol for NPK sensor communication

## Hardware Requirements

- **Microcontroller**: ESP32 (with WiFi & dual UART support)
- **Sensors**:
  - NPK Sensor (Modbus RTU via RS485)
  - Capacitive/Analog Soil Moisture Sensor
- **Display**: SSD1306 OLED (128x64) via I2C
- **Actuator**: 5V Relay Module
- **Communication**: RS485 Transceiver Module
- **Power**: USB or 5V supply

## Pin Configuration

| Component | GPIO Pin | Details |
|-----------|----------|---------|
| OLED SDA | 21 | I2C Data |
| OLED SCL | 22 | I2C Clock |
| RS485 RX | 26 | Serial2 |
| RS485 TX | 27 | Serial2 |
| RS485 DE | 4 | Driver Enable |
| RS485 RE | 5 | Receiver Enable |
| Soil Moisture | 32 | Analog Input (ADC) |
| Water Pump Relay | 15 | Digital Output |

## Installation

### 1. Dependencies
Install required libraries via Arduino IDE:
- `Adafruit GFX`
- `Adafruit SSD1306`
- `WiFi` (built-in)
- `UniversalTelegramBot`
- `ArduinoJson`

### 2. Configuration
Edit the following in [NPK_Soil_Telegram_Pump.ino](NPK_Soil_Telegram_Pump.ino):

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* botToken = "YOUR_BOT_TOKEN";
const char* chatID = "YOUR_CHAT_ID";
```

### 3. Calibration
Adjust soil moisture calibration values based on your sensor:

```cpp
int rawDry = 3700;   // Raw value when dry (in air)
int rawWet = 1600;   // Raw value when wet (in water)
```

### 4. Upload
Upload the sketch to your ESP32 board using Arduino IDE.

## Configuration Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| `botRequestDelay` | 60000 ms | Telegram report interval |
| `wifiCheckInterval` | 20000 ms | WiFi reconnection check |
| `TH_LOW` | 35% | Moisture threshold to turn pump ON |
| `TH_HIGH` | 60% | Moisture threshold to turn pump OFF |
| `relayActiveLow` | true | Relay trigger logic (LOW or HIGH) |

## How It Works

### Pump Control Logic (Hysteresis)
- **Moisture < 35%**: Pump turns ON
- **Moisture > 60%**: Pump turns OFF
- **Between 35-60%**: Pump maintains previous state (prevents relay chatter)

### NPK Data Reading
Uses Modbus RTU protocol over RS485 to query:
- Nitrogen (N) - mg/kg
- Phosphorus (P) - mg/kg
- Potassium (K) - mg/kg

### Telegram Reporting
Sends formatted status report every 1 minute containing:
- Soil moisture percentage
- Pump operating status
- Current NPK values

## OLED Display Output

```
WiFi:OK | Pump:ON
Moist: 45%
N: 120 mg
P: 85 mg
K: 200 mg
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Display not showing | Check I2C address (0x3C), verify SDA/SCL connections |
| NPK values always 0 | Verify RS485 DE/RE pins, check baud rate (4800), test Modbus commands |
| Pump not responding | Confirm PIN_RELAY (15), test relay logic with `relayActiveLow` |
| WiFi disconnects | Check WiFi credentials, verify signal strength |
| Moisture reading wrong | Recalibrate `rawDry` and `rawWet` values |

## Telegram Bot Setup

1. Create a bot via [@BotFather](https://t.me/botfather) on Telegram
2. Get your Chat ID from [@userinfobot](https://t.me/userinfobot)
3. Update `botToken` and `chatID` in the sketch
4. Start a conversation with your bot

## Serial Monitor Output

```
Moist: 45% (2500) | Pump: ON | N: 120 P: 85 K: 200
Telegram Sent Successfully
```

## License

MIT License - Feel free to use and modify for your projects.

## Support

For issues or questions, check:
- Arduino IDE Serial Monitor for debug output
- OLED display for real-time status
- Telegram messages for report validation
