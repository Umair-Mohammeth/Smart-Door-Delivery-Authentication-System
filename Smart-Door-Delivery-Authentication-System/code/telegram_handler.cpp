#include "telegram_handler.h"
#include "config.h"
#include "main.h"
#include "rfid_handler.h"
#include "camera_handler.h"

void handleTelegramMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized access denied. This event has been logged.", "");
      Serial.println("Unauthorized Telegram access attempt from: " + from_name + " (" + chat_id + ")");
      continue;
    }

    Serial.println("Telegram command received: " + text);

    if (text == "/start" || text == "/help") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Smart Door Delivery Authentication System\n\n";
      welcome += "/open - Unlock the door remotely\n";
      welcome += "/photo - Take a real-time photo\n";
      welcome += "/status - Check system status\n";
      welcome += "/adduid [UID] - Add authorized RFID UID\n";
      welcome += "/deluid [UID] - Remove RFID UID\n";
      bot.sendMessage(chat_id, welcome, "");
    } else if (text == "/open") {
      unlockDoor();
      bot.sendMessage(chat_id, "🔓 Door unlocked remotely.", "");
      captureImage("remote_open");
    } else if (text == "/photo") {
      bot.sendMessage(chat_id, "📸 Capturing real-time photo...", "");
      captureImage("manual_request");
    } else if (text == "/status") {
      String status = "✅ System Online\n";
      status += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
      status += "Uptime: " + String(millis() / 60000) + " minutes";
      bot.sendMessage(chat_id, status, "");
    } else if (text.startsWith("/adduid ")) {
      String uid = text.substring(8);
      addUid(uid);
    } else if (text.startsWith("/deluid ")) {
      String uid = text.substring(8);
      removeUid(uid);
    } else {
      bot.sendMessage(chat_id, "Unknown command. Send /help for a list of commands.", "");
    }
  }
}
