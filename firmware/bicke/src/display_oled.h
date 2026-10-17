#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>

void initOLED();
void tampilOLED(String baris1, String baris2 = "", String baris3 = "");
void tampilParkirAktif(unsigned long durasi, unsigned long biaya);
void tampilParkirSelesai(unsigned long durasi, unsigned long biaya);

#endif