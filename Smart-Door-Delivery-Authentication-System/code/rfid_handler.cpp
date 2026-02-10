#include "rfid_handler.h"
#include "config.h"
#include "main.h"
#include "camera_handler.h"
#include "FS.h"
#include "SD_MMC.h"

void handleRfid() {
  if (isUidAuthorized(mfrc522.uid.uidByte)) {
    unlockDoor();
    captureImage("delivery");
    bot.sendMessage(CHAT_ID, "Delivery person authenticated.", "");
  } else {
    bot.sendMessage(CHAT_ID, "Unauthorized RFID scan detected.", "");
  }
  mfrc522.PICC_HaltA();
}

bool isUidAuthorized(byte* uid) {
  File file = SD_MMC.open(UID_FILE);
  if (!file) {
    return false;
  }

  String uidStr = "";
  for (byte i = 0; i < UID_LENGTH; i++) {
    uidStr += String(uid[i] < 0x10 ? "0" : "");
    uidStr += String(uid[i], HEX);
  }
  uidStr.toUpperCase();

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (uidStr.equals(line)) {
      file.close();
      return true;
    }
  }

  file.close();
  return false;
}

void addUid(String uid) {
  uid.trim();
  uid.toUpperCase();
  File file = SD_MMC.open(UID_FILE, FILE_APPEND);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file.", "");
    return;
  }
  file.println(uid);
  file.close();
  bot.sendMessage(CHAT_ID, "UID added.", "");
}

void removeUid(String uid) {
  uid.trim();
  uid.toUpperCase();
  File file = SD_MMC.open(UID_FILE, FILE_READ);
  if (!file) {
    bot.sendMessage(CHAT_ID, "Failed to open UID file.", "");
    return;
  }

  String tempFileContent = "";
  bool found = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.equals(uid)) {
      found = true;
    } else {
      tempFileContent += line + "\n";
    }
  }
  file.close();

  if (found) {
    file = SD_MMC.open(UID_FILE, FILE_WRITE);
    file.print(tempFileContent);
    file.close();
    bot.sendMessage(CHAT_ID, "UID removed.", "");
  } else {
    bot.sendMessage(CHAT_ID, "UID not found.", "");
  }
}
