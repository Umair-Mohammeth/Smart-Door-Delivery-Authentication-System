#include "rfid_handler.h"
#include "config.h"
#include "main.h"
#include "camera_handler.h"
#include "FS.h"
#include "LittleFS.h"

void handleRfid() {
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidStr += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  Serial.print("RFID Scan: ");
  Serial.println(uidStr);

  if (isUidAuthorized(mfrc522.uid.uidByte)) {
    Serial.println("Authorized Access!");
    unlockDoor();
    bot.sendMessage(CHAT_ID, "Authorized delivery: " + uidStr, "");
    captureAndSendPhoto("delivery_auth");
  } else {
    Serial.println("Unauthorized Access!");
    bot.sendMessage(CHAT_ID, "Unauthorized RFID scan: " + uidStr, "");
    captureAndSendPhoto("delivery_unauth");
  }
  mfrc522.PICC_HaltA();
}

bool isUidAuthorized(byte* uid) {
  File file = LittleFS.open(UID_FILE, FILE_READ);
  if (!file) {
    Serial.println("Failed to open UID file for reading.");
    return false;
  }

  String uidStr = "";
  for (byte i = 0; i < UID_LENGTH; i++) {
    uidStr += String(uid[i] < 0x10 ? "0" : "");
    uidStr += String(uid[i], HEX);
  }
  uidStr.toUpperCase();

  bool authorized = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    if (uidStr.equals(line)) {
      authorized = true;
      break;
    }
  }

  file.close();
  return authorized;
}

void addUid(String uid) {
  uid.trim();
  uid.toUpperCase();
  File file = LittleFS.open(UID_FILE, FILE_APPEND);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file for appending.", "");
    return;
  }
  file.println(uid);
  file.close();
  bot.sendMessage(CHAT_ID, "UID " + uid + " added.", "");
}

void removeUid(String uid) {
  uid.trim();
  uid.toUpperCase();
  File file = LittleFS.open(UID_FILE, FILE_READ);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file.", "");
    return;
  }

  String tempFileContent = "";
  bool found = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    if (line.equals(uid)) {
      found = true;
    } else {
      tempFileContent += line + "\n";
    }
  }
  file.close();

  if (found) {
    file = LittleFS.open(UID_FILE, FILE_WRITE);
    file.print(tempFileContent);
    file.close();
    bot.sendMessage(CHAT_ID, "UID " + uid + " removed.", "");
  } else {
    bot.sendMessage(CHAT_ID, "UID " + uid + " not found.", "");
  }
}
