#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#include "app_firebase.h"
#include "config.h"
#include "sensors.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig firebaseConfig;

String documentPath = "locks/" + String(BIKE_ID);

extern bool parkirAktif;
extern String activeSource;

extern void mulaiParkirDariApp();
extern void selesaiParkirDariApp();

void initFirebaseApp() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi terhubung");

  firebaseConfig.api_key = FIREBASE_API_KEY;
  auth.user.email = FIREBASE_USER_EMAIL;
  auth.user.password = FIREBASE_USER_PASSWORD;

  Firebase.begin(&firebaseConfig, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase siap");
}

void updateFirestoreString(String field, String value) {
  FirebaseJson content;
  String path = "fields/" + field + "/stringValue";
  content.set(path.c_str(), value);

  Firebase.Firestore.patchDocument(
    &fbdo, FIREBASE_PROJECT_ID, "",
    documentPath.c_str(), content.raw(), field.c_str()
  );
}

void updateFirestoreBool(String field, bool value) {
  FirebaseJson content;
  String path = "fields/" + field + "/booleanValue";
  content.set(path.c_str(), value);

  Firebase.Firestore.patchDocument(
    &fbdo, FIREBASE_PROJECT_ID, "",
    documentPath.c_str(), content.raw(), field.c_str()
  );
}

void updateFirestoreNull(String field) {
  FirebaseJson content;
  String path = "fields/" + field + "/nullValue";
  content.set(path.c_str(), "NULL_VALUE");

  Firebase.Firestore.patchDocument(
    &fbdo, FIREBASE_PROJECT_ID, "",
    documentPath.c_str(), content.raw(), field.c_str()
  );
}

void clearCommand() {
  updateFirestoreString("command", "none");
}

void setFirebaseActiveSource(String source) {
  updateFirestoreString("activeSource", source);
}

void setFirebaseUsageStatus(String status) {
  updateFirestoreString("usageStatus", status);
}

void setFirebaseLockStatus(String status) {
  updateFirestoreString("lockStatus", status);
}

void setFirebasePaid(bool paid) {
  updateFirestoreBool("paid", paid);
}

void setFirebaseActiveRfidUid(String uid) {
  updateFirestoreString("activeRfidUid", uid);
}

void setFirebaseActiveUserId(String userId) {
  updateFirestoreString("activeUserId", userId);
}

void setFirebaseUserType(String userType) {
  updateFirestoreString("userType", userType);
}

void setFirebaseActiveHistoryId(String historyId) {
  updateFirestoreString("activeHistoryId", historyId);
}

void resetFirebaseLockSession() {
  setFirebaseActiveSource("none");
  setFirebaseUsageStatus("available");
  setFirebaseLockStatus("unlocked");
  setFirebasePaid(false);
  setFirebaseActiveRfidUid("");
  setFirebaseActiveUserId("");
  setFirebaseUserType("");
  setFirebaseActiveHistoryId("");
  updateFirestoreNull("startTime");
  clearCommand();
}

void updateFirebaseSensorStatus() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate < 2000) return;
  lastUpdate = millis();

  if (!Firebase.ready()) return;

  updateFirestoreBool("infraredDetected", sepedaTerdeteksi());
  updateFirestoreBool("limitSwitchPressed", kunciTerkunci());
}

void checkAppCommand() {
  if (!Firebase.ready()) return;

  if (Firebase.Firestore.getDocument(
        &fbdo,
        FIREBASE_PROJECT_ID,
        "",
        documentPath.c_str()
      )) {

    FirebaseJson json;
    FirebaseJsonData data;

    json.setJsonData(fbdo.payload());
    json.get(data, "fields/command/stringValue");

    if (!data.success) return;

    String command = data.stringValue;

    if (command == "lock" || command == "unlock") {
      if (!parkirAktif && activeSource == "none") {
        activeSource = "app";

        setFirebaseActiveSource("app");
        setFirebaseUsageStatus("in_use");
        setFirebaseLockStatus("locking");
        setFirebaseActiveRfidUid("");
        setFirebaseActiveUserId("app_user");
        setFirebaseUserType("non_member");
        setFirebasePaid(true);

        mulaiParkirDariApp();
      }

      clearCommand();
    }

    else if (command == "end_trip") {
      if (parkirAktif && activeSource == "app") {
        setFirebaseLockStatus("unlocking");

        selesaiParkirDariApp();

        activeSource = "none";
        resetFirebaseLockSession();
      }

      clearCommand();
    }
  } else {
    Serial.print("Firestore Error: ");
    Serial.println(fbdo.errorReason());
  }
}

String getRFIDUserTypeFromFirebase(String cardUid) {
  if (!Firebase.ready()) return "invalid";

  String rfidPath = "rfidCards/" + cardUid;

  if (Firebase.Firestore.getDocument(
        &fbdo,
        FIREBASE_PROJECT_ID,
        "",
        rfidPath.c_str()
      )) {

    FirebaseJson json;
    FirebaseJsonData data;

    json.setJsonData(fbdo.payload());

    json.get(data, "fields/status/stringValue");

    if (!data.success) {
      Serial.println("Field status RFID tidak ditemukan");
      return "invalid";
    }

    String status = data.stringValue;

    if (status != "active") {
      Serial.println("RFID tidak aktif");
      return "invalid";
    }

    json.get(data, "fields/userType/stringValue");

    if (!data.success) {
      Serial.println("Field userType tidak ditemukan, default member");
      return "member";
    }

    String userType = data.stringValue;

    if (userType == "member") {
      Serial.println("RFID terdaftar sebagai MEMBER");
      return "member";
    }

    if (userType == "non_member") {
      Serial.println("RFID terdaftar sebagai NON MEMBER");
      return "non_member";
    }

    Serial.println("userType RFID tidak valid");
    return "invalid";
  }

  Serial.println("RFID tidak terdaftar");
  return "invalid";
}

bool checkRFIDCardFromFirebase(String cardUid) {
  String userType = getRFIDUserTypeFromFirebase(cardUid);

  if (userType == "member") {
    return true;
  }

  return false;
}

void saveHistoryToFirebase(
  String source,
  String userType,
  String userId,
  String ownerName,
  String rfidUid,
  unsigned long durationSeconds,
  unsigned long totalPrice
) {
  if (!Firebase.ready()) return;

  FirebaseJson content;

  unsigned long durationMinutes = (durationSeconds + 59) / 60;
  unsigned long ratePerHour = 0;

  if (userType == "non_member") {
    ratePerHour = TARIF_PER_JAM;
  }

  content.set("fields/lockId/stringValue", String(BIKE_ID));
  content.set("fields/source/stringValue", source);
  content.set("fields/userType/stringValue", userType);
  content.set("fields/userId/stringValue", userId);
  content.set("fields/ownerName/stringValue", ownerName);
  content.set("fields/rfidUid/stringValue", rfidUid);

  content.set("fields/durationSeconds/integerValue", String(durationSeconds));
  content.set("fields/durationMinutes/integerValue", String(durationMinutes));
  content.set("fields/ratePerHour/integerValue", String(ratePerHour));
  content.set("fields/totalPrice/integerValue", String(totalPrice));

  content.set("fields/status/stringValue", "completed");

  if (Firebase.Firestore.createDocument(
        &fbdo,
        FIREBASE_PROJECT_ID,
        "",
        "history",
        content.raw()
      )) {
    Serial.println("History berhasil disimpan ke Firebase");
  } else {
    Serial.print("Gagal simpan history: ");
    Serial.println(fbdo.errorReason());
  }
}