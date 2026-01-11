#include "main.h"
#include "config.h"
#include "rfid_handler.h"
#include "camera_handler.h"
#include "telegram_handler.h"
#include "tamper_handler.h"
#include <SPI.h>
#include <HX711.h>
#include <WiFi.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Servo.h>

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

  // Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized.");

  // Initialize HX711
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(HX711_CALIBRATION_FACTOR); // Placeholder for calibration
  scale.tare();      // Reset the scale to 0
  Serial.println("HX711 initialized and calibrated.");

  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("SD card initialized.");

  // Initialize Camera
  camera_init();
  Serial.println("Camera initialized.");

  // Initialize Servo
  servo.attach(SERVO_PIN);
  servo.write(SERVO_LOCKED_POS); // Lock the door
  Serial.println("Servo initialized.");

  setupTamper();

  bot.sendMessage(CHAT_ID, "Smart Door System Online", "");
}

void loop() {
  unsigned long currentTime = millis();

  handleTamper();

  if (doorUnlockTime > 0 && currentTime - doorUnlockTime >= doorUnlockDuration) {
    servo.write(SERVO_LOCKED_POS); // Lock the door
    doorUnlockTime = 0;
  }

  if (currentTime - lastExecutionTime >= executionInterval) {
    lastExecutionTime = currentTime;

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      handleRfid();
    }

    if (scale.is_ready()) {
      handleWeight();
    }

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    handleTelegramMessages(numNewMessages);
  }
}

void handleWeight() {
  long reading = scale.read();
  Serial.print("HX711 reading: ");
  Serial.println(reading);
  if (reading > PARCEL_DETECTION_THRESHOLD) { // Threshold for parcel presence
    bot.sendMessage(CHAT_ID, "Parcel detected on the doorstep.", "");
  }
}

void unlockDoor() {
  Serial.println("Unlocking door...");
  servo.write(SERVO_UNLOCKED_POS); // Unlock
  doorUnlockTime = millis();
}
