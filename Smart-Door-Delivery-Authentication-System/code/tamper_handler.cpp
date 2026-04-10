#include "tamper_handler.h"
#include "config.h"
#include "main.h"
#include "camera_handler.h"

unsigned long lastTamperAlert = 0;

void setupTamper() {
  pinMode(TAMPER_PIN, INPUT_PULLUP);
}

void handleTamper() {
  if (digitalRead(TAMPER_PIN) == LOW) { // Assuming LOW means vibration detected
    unsigned long currentTime = millis();
    if (currentTime - lastTamperAlert >= tamperAlertInterval) {
      lastTamperAlert = currentTime;
      Serial.println("Tamper detected!");
      bot.sendMessage(CHAT_ID, "🚨 Tampering detected at the door!", "");
      captureImage("tamper_alert");
    }
  }
}
