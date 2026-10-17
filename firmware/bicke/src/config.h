#ifndef CONFIG_H
#define CONFIG_H


#define RFID_SS    5
#define RFID_RST   4
#define RFID_SCK   21
#define RFID_MISO  22
#define RFID_MOSI  23


#define OLED_SDA    18
#define OLED_SCL    19
#define OLED_ADDR   0x3C

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64


#define SERVO_PIN   13


#define IR_PIN      12
#define LIMIT_PIN   14


#define TARIF_PER_JAM 10000

#define WIFI_SSID      "Galaxy A15444"
#define WIFI_PASSWORD  "33331111"


#define FIREBASE_API_KEY       "AIzaSyDKVO7BU0vpwY89cAq4a8RifAb0T4A-6dU"
#define FIREBASE_PROJECT_ID    "bicke-e886d"
#define FIREBASE_USER_EMAIL    "esp32@gmail.com"
#define FIREBASE_USER_PASSWORD "12345678"


#define BIKE_ID "bike_001"

#endif