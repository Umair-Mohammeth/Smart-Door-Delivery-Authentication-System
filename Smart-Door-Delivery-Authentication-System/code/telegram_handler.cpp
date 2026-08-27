#include "telegram_handler.h"
#include "main.h"
#include "rfid_handler.h"
#include "camera_handler.h"
#include "config.h"

void handleTelegramMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized access attempt.", "");
      continue;
    }

    if (text == "/start") {
      String welcome = "Welcome to Smart Door System.\n\n";
      welcome += "/open : Unlock the door\n";
      welcome += "/photo : Take a photo\n";
      welcome += "/status : System status\n";
      welcome += "/adduid [UID] : Add authorized UID\n";
      welcome += "/deluid [UID] : Remove authorized UID\n";
      bot.sendMessage(chat_id, welcome, "");
    } else if (text == "/open") {
      unlockDoor();
      bot.sendMessage(chat_id, "Door unlocked.", "");
      captureAndSendPhoto("remote_open");
    } else if (text == "/photo") {
      captureAndSendPhoto("manual");
    } else if (text == "/status") {
      bot.sendMessage(chat_id, "System is online and monitoring.", "");
    } else if (text.startsWith("/adduid ")) {
      String uid = text.substring(8);
      uid.trim();
      uid.toUpperCase();
      addUid(uid);
    } else if (text.startsWith("/deluid ")) {
      String uid = text.substring(8);
      uid.trim();
      uid.toUpperCase();
      removeUid(uid);
    } else {
      bot.sendMessage(chat_id, "Unknown command. Try /start for help.", "");
    }
  }
}
