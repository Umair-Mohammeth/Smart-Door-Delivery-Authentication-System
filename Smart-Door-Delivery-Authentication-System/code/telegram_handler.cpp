#include "telegram_handler.h"
#include "main.h"
#include "rfid_handler.h"
#include "camera_handler.h"

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
      captureImage("remote");
      bot.sendMessage(chat_id, "Door unlocked.", "");
    } else if (text == "/status") {
      bot.sendMessage(chat_id, "System is running.", "");
    } else if (text.startsWith("/adduid ")) {
      String uid = text.substring(8);
      addUid(uid);
    } else if (text.startsWith("/deluid ")) {
      String uid = text.substring(8);
      removeUid(uid);
    }
  }
}
