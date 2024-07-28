#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  ArduinoOTA.setHostname("esp-ota");
  ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
    String type;
    if(ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
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
  ArduinoOTA.handle();
}
