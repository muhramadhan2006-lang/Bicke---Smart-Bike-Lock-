#include <Arduino.h>
#include "sensors.h"
#include "config.h"

void initSensors() {
  pinMode(IR_PIN, INPUT);
  pinMode(LIMIT_PIN, INPUT_PULLUP);
}

bool sepedaTerdeteksi() {
  return digitalRead(IR_PIN) == LOW;
}

bool kunciTerkunci() {
  return digitalRead(LIMIT_PIN) == LOW;
}