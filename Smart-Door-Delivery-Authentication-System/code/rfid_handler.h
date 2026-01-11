#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <MFRC522.h>

void handleRfid();
bool isUidAuthorized(byte* uid);
void addUid(String uid);
void removeUid(String uid);

#endif // RFID_HANDLER_H
