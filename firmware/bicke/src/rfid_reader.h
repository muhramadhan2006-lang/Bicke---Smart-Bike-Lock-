#ifndef RFID_READER_H
#define RFID_READER_H

#include <Arduino.h>

void initRFID();
bool rfidTerdeteksi();
void stopRFID();

String getRFIDUid();

#endif