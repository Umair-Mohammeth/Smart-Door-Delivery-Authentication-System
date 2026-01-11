#include <SPI.h>
#include <MFRC522.h>
#include <HX711.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <SD_MMC.h>
#include "esp_camera.h"
#include <Servo.h>
#include "config.h"

// Pin definitions
#define SS_PIN    13
#define RST_PIN   12
#define HX711_DOUT_PIN 2
#define HX711_SCK_PIN  4
#define SERVO_PIN 15

#define PARCEL_DETECTION_THRESHOLD 1000

const int SERVO_LOCKED_POS = 0;
const int SERVO_UNLOCKED_POS = 90;

unsigned long lastExecutionTime = 0;
const unsigned long executionInterval = 1000;
unsigned long doorUnlockTime = 0;
const unsigned long doorUnlockDuration = 1000;

// Initialize libraries
MFRC522 mfrc522(SS_PIN, RST_PIN);
HX711 scale;
Servo servo;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

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

  bot.sendMessage(CHAT_ID, "Smart Door System Online", "");
}

void loop() {
  unsigned long currentTime = millis();

  if (doorUnlockTime > 0 && currentTime - doorUnlockTime >= doorUnlockDuration) {
    servo.write(SERVO_LOCKED_POS); // Lock the door
    doorUnlockTime = 0;
  }

  if (currentTime - lastExecutionTime >= executionInterval) {
    lastExecutionTime = currentTime;

    // Main logic goes here

    // 1. Check for RFID tag
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      handleRfid();
    }

    // 2. Check for parcel weight
    if (scale.is_ready()) {
      handleWeight();
    }

    // 3. Check for Telegram messages
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    handleTelegramMessages(numNewMessages);

    // 4. Tamper detection (placeholder) is not implemented
  }
}

void handleRfid() {
  Serial.print("RFID UID: ");
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  // Authenticate RFID tag
  bool authorized = false;
  for (int i = 0; i < NUM_AUTHORIZED_UIDS; i++) {
    if (compareUID(mfrc522.uid.uidByte, authorizedUIDs[i])) {
      authorized = true;
      break;
    }
  }

  if (authorized) {
    unlockDoor();
    captureImage("delivery");
    bot.sendMessage(CHAT_ID, "Delivery person authenticated.", "");
  } else {
    bot.sendMessage(CHAT_ID, "Unauthorized RFID scan detected.", "");
  }
  mfrc522.PICC_HaltA();
}

void handleWeight() {
  long reading = scale.read();
  Serial.print("HX711 reading: ");
  Serial.println(reading);
  if (reading > PARCEL_DETECTION_THRESHOLD) { // Threshold for parcel presence
    bot.sendMessage(CHAT_ID, "Parcel detected on the doorstep.", "");
  }
}

void handleTelegramMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized access.", "");
      continue;
    }

    if (text == "/open") {
      unlockDoor();
      bot.sendMessage(chat_id, "Door unlocked.", "");
    } else if (text == "/status") {
      bot.sendMessage(chat_id, "System is running.", "");
    }
  }
}

bool compareUID(byte* uid1, byte* uid2) {
  for (int i = 0; i < UID_LENGTH; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

void unlockDoor() {
  // Servo logic to unlock the door
  Serial.println("Unlocking door...");
  servo.write(SERVO_UNLOCKED_POS); // Unlock
  doorUnlockTime = millis();
}

void captureImage(String eventType) {
  // Camera logic to capture and save image to SD card
  Serial.println("Capturing image...");
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) {
    String path = "/" + eventType + "_" + String(millis()) + ".jpg";
    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_WRITE);
    if(file){
      file.write(fb->buf, fb->len);
      bot.sendMessage(CHAT_ID, "Image captured and saved.", "");
    }
    esp_camera_fb_return(fb);
  }
}

void camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
