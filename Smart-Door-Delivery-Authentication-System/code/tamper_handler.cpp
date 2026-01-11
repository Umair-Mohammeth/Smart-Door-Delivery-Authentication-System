#include "tamper_handler.h"
#include "config.h"
#include "main.h"

unsigned long lastTamperAlert = 0;

void setupTamper() {
  pinMode(TAMPER_PIN, INPUT_PULLUP);
}

void handleTamper() {
  if (digitalRead(TAMPER_PIN) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastTamperAlert >= tamperAlertInterval) {
      lastTamperAlert = currentTime;
      bot.sendMessage(CHAT_ID, "Tampering detected!", "");
    }
  }
}
