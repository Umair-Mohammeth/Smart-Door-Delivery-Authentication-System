#include "tamper_handler.h"
#include "config.h"
#include "main.h"
#include "camera_handler.h"

unsigned long lastTamperAlert = 0;

void setupTamper() {
  pinMode(TAMPER_PIN, INPUT_PULLUP);
}

void handleTamper() {
  // Tamper sensor triggers on LOW (vibration/displacement)
  if (digitalRead(TAMPER_PIN) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastTamperAlert >= tamperAlertInterval) {
      lastTamperAlert = currentTime;
      Serial.println("Tampering Detected!");
      bot.sendMessage(CHAT_ID, "⚠️ Alert: Tampering detected!", "");
      captureAndSendPhoto("tamper");
    }
  }
}
