#include "main.h"
#include "config.h"
#include "rfid_handler.h"
#include "camera_handler.h"
#include "telegram_handler.h"
#include "tamper_handler.h"
#include <SPI.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include <SD_MMC.h>
#include <ESP32Servo.h>

// Initialize libraries
MFRC522 mfrc522(SS_PIN, RST_PIN);
HX711 scale;
Servo doorServo;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastExecutionTime = 0;
unsigned long doorUnlockTime = 0;
bool parcelDetected = false;
unsigned long parcelDetectionStartTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Smart Door Delivery Authentication System ---");

  // Initialize WiFi and Telegram
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  client.setInsecure(); // Required for Telegram Bot API on ESP32

  // Initialize SD Card in 1-bit mode to free up GPIOs 4, 12, 13
  if(!SD_MMC.begin("/sdcard", true)){
    Serial.println("SD Card Mount Failed");
  } else {
    Serial.println("SD Card initialized.");
  }

  // Initialize SPI for RFID (sharing bus with SD Card)
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, SS_PIN);
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized.");

  // Initialize HX711
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(HX711_CALIBRATION_FACTOR);
  scale.tare();
  Serial.println("HX711 initialized.");

  // Initialize Camera
  camera_init();
  Serial.println("Camera initialized.");

  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(SERVO_LOCKED_POS);
  Serial.println("Servo initialized.");

  setupTamper();

  bot.sendMessage(CHAT_ID, "Smart Door System Online", "");
}

void loop() {
  unsigned long currentTime = millis();

  handleTamper();

  // Handle door locking timeout
  if (doorUnlockTime > 0 && currentTime - doorUnlockTime >= doorUnlockDuration) {
    Serial.println("Auto-locking door.");
    doorServo.write(SERVO_LOCKED_POS);
    doorUnlockTime = 0;
  }

  // Periodic sensor and message checks
  if (currentTime - lastExecutionTime >= executionInterval) {
    lastExecutionTime = currentTime;

    // Check RFID
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      handleRfid();
    }

    // Check Weight
    if (scale.is_ready()) {
      handleWeight();
    }

    // Check Telegram Messages
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}

void handleWeight() {
  long reading = scale.get_units(5); // Average of 5 readings

  if (reading > PARCEL_DETECTION_THRESHOLD) {
    if (!parcelDetected) {
      if (parcelDetectionStartTime == 0) {
        parcelDetectionStartTime = millis();
      } else if (millis() - parcelDetectionStartTime >= parcelDebounceTime) {
        parcelDetected = true;
        Serial.println("Parcel detected!");
        bot.sendMessage(CHAT_ID, "📦 Parcel detected on the doorstep.", "");
        captureImage("parcel_alert");
      }
    }
  } else {
    if (parcelDetected) {
      Serial.println("Parcel removed.");
      bot.sendMessage(CHAT_ID, "ℹ️ Parcel has been removed from the doorstep.", "");
    }
    parcelDetected = false;
    parcelDetectionStartTime = 0;
  }
}

void unlockDoor() {
  Serial.println("Unlocking door...");
  doorServo.write(SERVO_UNLOCKED_POS);
  doorUnlockTime = millis();
}
