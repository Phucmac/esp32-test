#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  17

Adafruit_ST7735 tft = Adafruit_ST7735(
  TFT_CS,
  TFT_DC,
  TFT_RST
);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 SPI LCD Test");

  tft.initR(INITR_BLACKTAB);   
  tft.setRotation(1);          
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.println("HELLO");

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 50);
  tft.println("ESP32 + SPI LCD");

  delay(1000);
}

void loop() {
  static int count = 0;

  tft.fillRect(10, 80, 120, 20, ST77XX_BLACK);
  tft.setCursor(10, 80);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.print(count++);

  delay(1000);
}