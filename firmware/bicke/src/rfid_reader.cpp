#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "rfid_reader.h"
#include "config.h"

MFRC522 mfrc522(RFID_SS, RFID_RST);

void initRFID() {
  
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);

  mfrc522.PCD_Init();

  Serial.println("RFID siap digunakan");
}

bool rfidTerdeteksi() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  Serial.println("\nRFID TERDETEKSI");

  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  return true;
}

String getRFIDUid() {
  String uid = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(mfrc522.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  return uid;
}

void stopRFID() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}