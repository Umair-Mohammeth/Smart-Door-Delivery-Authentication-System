#ifndef MAIN_H
#define MAIN_H

#include <UniversalTelegramBot.h>
#include <MFRC522.h>
#include <WiFiClientSecure.h>

// Extern declarations for global objects
extern UniversalTelegramBot bot;
extern MFRC522 mfrc522;
extern WiFiClientSecure client;

// Function prototypes from main.ino
void unlockDoor();
void handleWeight();

#endif // MAIN_H
