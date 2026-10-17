#include <Arduino.h>

#include "config.h"
#include "display_oled.h"
#include "servo_lock.h"
#include "sensors.h"
#include "rfid_reader.h"
#include "app_firebase.h"

bool parkirAktif = false;
String activeSource = "none";
String activeRfidUid = "";
String userTypeAktif = "";

unsigned long waktuMulai = 0;
unsigned long durasi = 0;
unsigned long biaya = 0;

unsigned long hitungBiaya(unsigned long durasiDetik, String userType) {
  if (userType == "member") return 0;

  unsigned long durasiJam = (durasiDetik + 3599) / 3600;
  if (durasiJam < 1) durasiJam = 1;

  return durasiJam * TARIF_PER_JAM;
}

void mulaiParkir(String sumber, String userType = "non_member") {
  if (sumber == "rfid") {
    if (userType == "member") {
      tampilOLED("RFID MEMBER", "GRATIS");
    } else {
      tampilOLED("RFID NON MEMBER", "Rp10K / Jam");
    }
  } else {
    tampilOLED("APP ACCESS", "Rp10K / Jam");
  }

  delay(1500);

  if (!sepedaTerdeteksi()) {
    tampilOLED("AKSES DITOLAK", "Sepeda tidak", "terdeteksi");

    parkirAktif = false;
    activeSource = "none";
    activeRfidUid = "";
    userTypeAktif = "";

    resetFirebaseLockSession();
    setFirebaseUsageStatus("available");
    setFirebaseLockStatus("unlocked");

    delay(2000);
    tampilOLED("BICKE READY", "Gembok terbuka", "Scan App/RFID");
    return;
  }

  tampilOLED("SEPEDA TERDETEKSI", "Servo menutup", "Mengunci...");
  delay(1000);

  tutupSampaiTerkunci();

  waktuMulai = millis();
  parkirAktif = true;
  activeSource = sumber;
  userTypeAktif = userType;

  setFirebaseActiveSource(sumber);
  setFirebaseUsageStatus("in_use");
  setFirebaseLockStatus("locked");
  setFirebaseUserType(userType);

  if (sumber == "rfid") {
    setFirebaseActiveRfidUid(activeRfidUid);

    if (userType == "member") {
      setFirebaseActiveUserId("rfid_member");
      setFirebasePaid(true);
    } else {
      setFirebaseActiveUserId("rfid_non_member");
      setFirebasePaid(false);
    }
  } else {
    setFirebaseActiveRfidUid("");
    setFirebaseActiveUserId("app_user");
    setFirebasePaid(true);
  }

  if (sumber == "rfid") {
    if (userType == "member") {
      tampilOLED("PARKIR AKTIF", "RFID MEMBER", "GRATIS");
    } else {
      tampilOLED("PARKIR AKTIF", "RFID NON MEMBER", "Rp10K/Jam");
    }
  } else {
    tampilOLED("PARKIR AKTIF", "Dari APP", "Rp10K/Jam");
  }
}

void selesaiParkir(String sumber) {
  durasi = (millis() - waktuMulai) / 1000;
  biaya = hitungBiaya(durasi, userTypeAktif);

  String userId = "guest";
  String ownerName = "Guest";
  String rfidUidForHistory = activeRfidUid;

  if (sumber == "app") {
    userId = "app_user";
    ownerName = "App User";
    rfidUidForHistory = "";
  } else if (userTypeAktif == "member") {
    userId = "rfid_member";
    ownerName = "RFID Member";
  } else {
    userId = "rfid_non_member";
    ownerName = "RFID Non Member";
  }

  saveHistoryToFirebase(
    sumber,
    userTypeAktif,
    userId,
    ownerName,
    rfidUidForHistory,
    durasi,
    biaya
  );

  tampilParkirSelesai(durasi, biaya);
  delay(3000);

  bukaKunci();

  parkirAktif = false;
  activeSource = "none";
  activeRfidUid = "";
  userTypeAktif = "";

  resetFirebaseLockSession();
  setFirebaseUsageStatus("available");
  setFirebaseLockStatus("unlocked");

  tampilOLED("BICKE READY", "Gembok terbuka", "Scan App/RFID");
}

void mulaiParkirDariApp() {
  mulaiParkir("app", "non_member");
}

void selesaiParkirDariApp() {
  selesaiParkir("app");
}

void setup() {
  Serial.begin(115200);

  initRFID();
  initSensors();
  initOLED();
  initServo();
  initFirebaseApp();

  resetFirebaseLockSession();

  parkirAktif = false;
  activeSource = "none";
  activeRfidUid = "";
  userTypeAktif = "";

  setFirebaseUsageStatus("available");
  setFirebaseLockStatus("unlocked");

  tampilOLED("BICKE READY", "Gembok terbuka", "Scan App/RFID");

  Serial.println("=================================");
  Serial.println("          BICKE READY");
  Serial.println("=================================");
  Serial.println("Kondisi awal: gembok terbuka");
  Serial.println("Servo tidak bergerak saat startup");
  Serial.println("Tap RFID/App untuk mengunci");
}

void loop() {
  updateFirebaseSensorStatus();
  checkAppCommand();

  if (parkirAktif) {
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate >= 1000) {
      lastUpdate = millis();

      unsigned long durasiSekarang = (millis() - waktuMulai) / 1000;
      unsigned long biayaSekarang = hitungBiaya(durasiSekarang, userTypeAktif);

      tampilParkirAktif(durasiSekarang, biayaSekarang);
    }
  }

  if (!rfidTerdeteksi()) return;

  String uid = getRFIDUid();

  stopRFID();

  Serial.print("UID RFID : ");
  Serial.println(uid);

  String rfidUserType = getRFIDUserTypeFromFirebase(uid);

  if (!parkirAktif && activeSource == "none") {
    activeRfidUid = uid;

    if (rfidUserType == "member") {
      Serial.println("RFID MEMBER");
      mulaiParkir("rfid", "member");
    } else {
      Serial.println("RFID tidak terdaftar / non_member");
      mulaiParkir("rfid", "non_member");
    }
  }

  else if (parkirAktif && activeSource == "rfid") {
    if (uid != activeRfidUid) {
      tampilOLED("RFID DITOLAK", "Gunakan kartu", "yang sama");
      delay(1000);
      return;
    }

    selesaiParkir("rfid");
  }

  else if (parkirAktif && activeSource == "app") {
    tampilOLED("AKSES DITOLAK", "Sedang dipakai", "oleh APP");
    delay(1000);
  }

  delay(1000);
} 