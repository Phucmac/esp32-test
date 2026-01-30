#include <Arduino.h>
#include "DHT.h"
#define DHTPIN 4        
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 DevKit V1 + DHT11");
  dht.begin();          
}
void loop() {
  delay(2000);          

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Read DHT11 failed");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print(" °C | Humi: ");
  Serial.print(h);
  Serial.println(" %");
}