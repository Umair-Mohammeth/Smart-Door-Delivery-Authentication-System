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
#include <LittleFS.h>
#include <ESP32Servo.h>

// Initialize libraries
MFRC522 mfrc522(SS_PIN, RST_PIN);
HX711 scale;
Servo servo;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastExecutionTime = 0;
unsigned long doorUnlockTime = 0;

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
  } else {
    Serial.println("LittleFS initialized.");
  }

  // Initialize RFID
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized.");

  // Initialize HX711
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(HX711_CALIBRATION_FACTOR);
  scale.tare();
  Serial.println("HX711 initialized.");

  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Camera
  camera_init();
  Serial.println("Camera initialized.");

  // Initialize Servo
  servo.attach(SERVO_PIN_ALT); // Using Pin 4
  servo.write(SERVO_LOCKED_POS);
  Serial.println("Servo initialized.");

  setupTamper();

  bot.sendMessage(CHAT_ID, "Smart Door System Online", "");
}

void loop() {
  unsigned long currentTime = millis();

  handleTamper();

  if (doorUnlockTime > 0 && currentTime - doorUnlockTime >= doorUnlockDuration) {
    servo.write(SERVO_LOCKED_POS);
    doorUnlockTime = 0;
    Serial.println("Door locked.");
  }

  if (currentTime - lastExecutionTime >= executionInterval) {
    lastExecutionTime = currentTime;

    // RESTORED: RFID Polling
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      handleRfid();
    }

    if (scale.is_ready()) {
      handleWeight();
    }

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages > 0) {
      handleTelegramMessages(numNewMessages);
    }
  }
}

void handleWeight() {
  static bool parcelDetected = false;
  long reading = scale.get_units(5);

  if (reading > PARCEL_DETECTION_THRESHOLD && !parcelDetected) {
    parcelDetected = true;
    bot.sendMessage(CHAT_ID, "Parcel detected on the doorstep.", "");
    captureAndSendPhoto("parcel");
  } else if (reading < (PARCEL_DETECTION_THRESHOLD / 2) && parcelDetected) {
    parcelDetected = false;
    bot.sendMessage(CHAT_ID, "Parcel removed.", "");
  }
}

void unlockDoor() {
  Serial.println("Unlocking door...");
  servo.write(SERVO_UNLOCKED_POS);
  doorUnlockTime = millis();
}
