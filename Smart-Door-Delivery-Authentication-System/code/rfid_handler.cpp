#include "rfid_handler.h"
#include "config.h"
#include "main.h"
#include "camera_handler.h"
#include "FS.h"
#include "SD_MMC.h"

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
    Serial.println("Access Granted.");
    bot.sendMessage(CHAT_ID, "✅ Delivery person authenticated (UID: " + uidStr + ").", "");
    unlockDoor();
    captureImage("delivery_auth");
  } else {
    Serial.println("Access Denied.");
    bot.sendMessage(CHAT_ID, "⚠️ Unauthorized RFID scan detected (UID: " + uidStr + ")!", "");
    captureImage("unauthorized_rfid");
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

bool isUidAuthorized(byte* uid) {
  if (!SD_MMC.exists(UID_FILE)) {
    Serial.println("UID file does not exist.");
    return false;
  }

  File file = SD_MMC.open(UID_FILE, FILE_READ);
  if (!file) {
    Serial.println("Failed to open UID file for reading.");
    return false;
  }

  String targetUid = "";
  for (byte i = 0; i < UID_LENGTH; i++) {
    targetUid += String(uid[i] < 0x10 ? "0" : "");
    targetUid += String(uid[i], HEX);
  }
  targetUid.toUpperCase();
  targetUid.trim();

  bool authorized = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    if (targetUid.equals(line)) {
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

  if (uid == "") {
    bot.sendMessage(CHAT_ID, "Invalid UID.", "");
    return;
  }

  File file = SD_MMC.open(UID_FILE, FILE_APPEND);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file for adding.", "");
    return;
  }
  file.println(uid);
  file.close();
  bot.sendMessage(CHAT_ID, "✅ UID " + uid + " added to authorized list.", "");
}

void removeUid(String uid) {
  uid.trim();
  uid.toUpperCase();

  if (!SD_MMC.exists(UID_FILE)) {
    bot.sendMessage(CHAT_ID, "UID file not found.", "");
    return;
  }

  File file = SD_MMC.open(UID_FILE, FILE_READ);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file for reading.", "");
    return;
  }

  String tempFileContent = "";
  bool found = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.equalsIgnoreCase(uid)) {
      found = true;
    } else if (line != "") {
      tempFileContent += line + "\n";
    }
  }
  file.close();

  if (found) {
    file = SD_MMC.open(UID_FILE, FILE_WRITE);
    if (file) {
      file.print(tempFileContent);
      file.close();
      bot.sendMessage(CHAT_ID, "✅ UID " + uid + " removed.", "");
    } else {
      bot.sendMessage(CHAT_ID, "Error updating UID file.", "");
    }
  } else {
    bot.sendMessage(CHAT_ID, "UID " + uid + " not found in list.", "");
  }
}
