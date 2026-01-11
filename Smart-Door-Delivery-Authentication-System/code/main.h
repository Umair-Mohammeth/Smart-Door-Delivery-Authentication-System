#ifndef MAIN_H
#define MAIN_H

#include <UniversalTelegramBot.h>
#include <MFRC522.h>

// Extern declarations for global objects
extern UniversalTelegramBot bot;
extern MFRC522 mfrc522;

// Function prototypes from main.ino
void unlockDoor();

#endif // MAIN_H
