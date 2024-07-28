#include <Arduino.h>
#include <ArduinoHA.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "OTAHandler.h"
#include "button.h"
#include "secrets.h"

#define HOSTNAME "toilet_fan"

void setupWiFi() {
  // template function
  delay(10);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  setupWiFi();

  // Логування розміру флеш-пам'яті
  uint32_t flashSize = ESP.getFlashChipRealSize();
  Serial.print("Flash size: ");
  Serial.print(flashSize);
  Serial.println(" bytes");

  // Ініціалізація OTA з паролем
  setupOTA(HOSTNAME, OTA_PASSWORD);
}

unsigned long lastUpdateAt = 0;             // Змінна для зберігання часу останнього оновлення
const unsigned long updateInterval = 3000;  // Інтервал оновлення в мілісекундах (3000 мс = 3 секунди)
unsigned int wifi_fail_counter = 0;
const unsigned int wifi_fail_triger = 300000;  // при кількості спроб реконнест
void loop() {
  ArduinoOTA.handle();
}
