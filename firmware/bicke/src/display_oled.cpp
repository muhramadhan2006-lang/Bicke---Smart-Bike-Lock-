#include "display_oled.h"
#include "config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED() {
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED gagal terdeteksi");
    while (true);
  }

  display.clearDisplay();
  display.display();

  tampilOLED("BICKE READY", "Gembok terbuka", "Scan App/RFID");
}

void tampilOLED(String baris1, String baris2, String baris3) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(baris1);

  display.setCursor(0, 18);
  display.println(baris2);

  display.setCursor(0, 36);
  display.println(baris3);

  display.display();
}

void tampilParkirAktif(unsigned long durasi, unsigned long biaya) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("PARKIR AKTIF");

  display.setCursor(0, 18);
  display.print("Durasi: ");
  display.print(durasi);
  display.println(" detik");

  display.setCursor(0, 36);
  display.print("Biaya : Rp ");
  display.println(biaya);

  display.setCursor(0, 54);
  display.println("Keluar via sumber sama");

  display.display();
}

void tampilParkirSelesai(unsigned long durasi, unsigned long biaya) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("PARKIR SELESAI");

  display.setCursor(0, 18);
  display.print("Durasi: ");
  display.print(durasi);
  display.println(" detik");

  display.setCursor(0, 36);
  display.print("Biaya : Rp ");
  display.println(biaya);

  display.display();
}