# Code Directory – Smart Door Delivery Authentication System

This folder contains the source code and firmware for the Smart Door Delivery Authentication System.

## 📜 Files Included

- `main.ino`:  
  The main Arduino sketch that:
  - Controls the ESP32-CAM module
  - Interfaces with RFID reader
  - Handles sensor input (load cell, IR)
  - Sends alerts using the Telegram bot
  - Manages SD card logging
  - Controls the locking mechanism (servo)

- `README.md`:  
  This file – explains the code structure and usage.

## 📌 Requirements

### Hardware:
- ESP32-CAM module
- RFID RC522 module
- Load Cell + HX711 amplifier
- IR sensor
- Servo motor or solenoid lock
- SD card module
- 5V power supply or USB power
- Breadboard and jumper wires

### Software:
- Arduino IDE with the following libraries:
  - `ESP32`
  - `MFRC522`
  - `HX711`
  - `UniversalTelegramBot`
  - `WiFi`
  - `FS` and `SD_MMC` for SD card handling

## Dependencies
This project relies on the following Arduino libraries. Please install them using the Arduino Library Manager:
- **MFRC522** by GitHubCommunity
- **HX711** by Bogdan Necula
- **UniversalTelegramBot** by Brian Lough
- **ESP32 Servo** by Kevin Harrington

## ⚙️ Setup Instructions
1. Install required libraries via Library Manager in Arduino IDE.
2. Configure Wi-Fi credentials and Telegram Bot token in the sketch.
3. Upload `main.ino` to the ESP32-CAM.
4. Connect components as per circuit diagram (in documentation).
5. Test the RFID scanning and verify Telegram message alerts.

## 🧪 Testing
- Unit-tested for each module (camera, RFID, load cell).
- Logs and captured images are stored in the SD card.

---

For code customization, refer to the comments within `main.ino`.
