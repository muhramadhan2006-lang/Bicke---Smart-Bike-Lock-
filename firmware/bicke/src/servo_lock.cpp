#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo_lock.h"
#include "config.h"
#include "display_oled.h"
#include "sensors.h"

Servo lockServo;

// =====================
// SETTING SERVO
// =====================
#define SERVO_BUKA   180
#define SERVO_TUTUP  0
#define SERVO_STOP   90

#define DELAY_BUKA   850
#define DELAY_TUTUP  910

void initServo() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);

  lockServo.write(SERVO_STOP);
}

void bukaKunci() {
  Serial.println("\n=== MEMBUKA GEMBOK ===");

  tampilOLED("MEMBUKA GEMBOK", "Mohon tunggu...");

  lockServo.write(SERVO_BUKA);
  delay(DELAY_BUKA);

  lockServo.write(SERVO_STOP);

  Serial.println("Gembok terbuka");

  tampilOLED("GEMBOK TERBUKA", "Siap digunakan");
}

void tutupKunci() {
  Serial.println("\n=== MENUTUP GEMBOK ===");

  tampilOLED("MENUTUP GEMBOK", "Mohon tunggu...");

  lockServo.write(SERVO_TUTUP);
  delay(DELAY_TUTUP);

  lockServo.write(SERVO_STOP);

  Serial.println("Servo selesai menutup");
}

void tutupSampaiTerkunci() {
  while (true) {
    tutupKunci();

    delay(500);

    if (kunciTerkunci()) {
      Serial.println("Limit switch aktif");
      Serial.println("Kunci berhasil terkunci");

      tampilOLED("KUNCI TERKUNCI", "Limit switch OK");
      delay(1500);

      return;
    }

    Serial.println("Limit switch belum aktif");
    Serial.println("Servo membuka kembali");

    tampilOLED("TUTUP GAGAL", "Buka kembali", "Tunggu 4 detik");

    bukaKunci();

    delay(4000);
  }
}