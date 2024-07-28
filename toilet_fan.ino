#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void setup() {
  // Ініціалізація серійного зв'язку для відлагодження
  Serial.begin(115200);
  Serial.println("Booting");

  // Підключення до WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Налаштування OTA
  ArduinoOTA.setHostname("esp8266-ota");
  ArduinoOTA.setPassword("admin");  // Встанови свій пароль

  ArduinoOTA.onStart([]() {
    String type;
    if(ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if(error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if(error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if(error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if(error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if(error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void loop() {
  // Обробка OTA запитів
  ArduinoOTA.handle();
}
