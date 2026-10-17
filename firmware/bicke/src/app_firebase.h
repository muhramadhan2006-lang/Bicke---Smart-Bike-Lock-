#ifndef APP_FIREBASE_H
#define APP_FIREBASE_H

#include <Arduino.h>

void initFirebaseApp();
void checkAppCommand();
void updateFirebaseSensorStatus();

void setFirebaseActiveSource(String source);
void setFirebaseUsageStatus(String status);
void setFirebaseLockStatus(String status);
void setFirebasePaid(bool paid);

void setFirebaseActiveRfidUid(String uid);
void setFirebaseActiveUserId(String userId);
void setFirebaseUserType(String userType);
void setFirebaseActiveHistoryId(String historyId);

void resetFirebaseLockSession();

bool checkRFIDCardFromFirebase(String cardUid);
String getRFIDUserTypeFromFirebase(String cardUid);

void saveHistoryToFirebase(
  String source,
  String userType,
  String userId,
  String ownerName,
  String rfidUid,
  unsigned long durationSeconds,
  unsigned long totalPrice
);

#endif