#include <Arduino.h>
#include <ArduinoHA.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "OTAHandler.h"
#include "button.h"
#include "secrets.h"

#define HOSTNAME "toilet_fan"
#define SSR_PIN 0
#define BUTTON_PIN 3  // only RX possible as input
#define LED 2
bool connected = false;

WiFiClient client;
HADevice device(HOSTNAME);
HAMqtt mqtt(client, device);
// Unique ID
HASwitch fanSwitch("fan_switch_toilet");
HASensorNumber wifiRssi("wifiRssi_toilet", HASensorNumber::PrecisionP0);

// Створюємо об'єкт кнопки
button btn(BUTTON_PIN);

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

void onSwitchCommand(bool state, HASwitch *sender) {
  // SSR only, instead MSR
  digitalWrite(SSR_PIN, (state ? HIGH : LOW));
  sender->setState(state);  // report state back to the Home Assistant
}

void onMqttConnected() {
  digitalWrite(LED, HIGH);
  connected = true;
}

void onMqttDisconnected() {
  Serial.println("Disconnected from the broker!");
  connected = false;
}

void onMqttStateChanged(HAMqtt::ConnectionState state) {
  Serial.print("MQTT state changed to: ");
  Serial.println(static_cast<int8_t>(state));
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

  // set device's details (optional)
  device.setName(HOSTNAME);
  device.setSoftwareVersion("1.0.0");

  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // set icon (optional)
  fanSwitch.setIcon("mdi:fan");
  fanSwitch.setName("Toilet Fan");

  // configure sensors
  wifiRssi.setIcon("mdi:wifi");
  wifiRssi.setName("WIFI RSSI");
  wifiRssi.setUnitOfMeasurement("dBm");

  // handle switch state
  fanSwitch.onCommand(onSwitchCommand);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  mqtt.onStateChanged(onMqttStateChanged);

  mqtt.begin(BROKER_ADDR, MQTT_USERNAME, MQTT_PASSWORD);
  device.enableSharedAvailability();
  device.enableLastWill();

  // Ініціалізація OTA з паролем
  setupOTA(HOSTNAME, OTA_PASSWORD);
}

unsigned long lastUpdateAt = 0;             // Змінна для зберігання часу останнього оновлення
const unsigned long updateInterval = 3000;  // Інтервал оновлення в мілісекундах (3000 мс = 3 секунди)
unsigned int wifi_fail_counter = 0;
const unsigned int wifi_fail_triger = 300000;  // при кількості спроб реконнест
void loop() {
  // Перевірка WiFi з'єднання
  if(!connected && WiFi.status() == WL_CONNECTED) {
    // not mqtt and connected to wifi
    Serial.println("WiFi OK, mqtt NOK");
    digitalWrite(LED, (millis() / 1000) % 2);
  }
  // Перевірка WiFi з'єднання
  if(WiFi.status() != WL_CONNECTED && wifi_fail_counter > wifi_fail_triger) {
    Serial.println("WiFi lost, trying to reconnect...");
    setupWiFi();
  }
  if(WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    wifi_fail_counter++;
    Serial.print("WiFi lost, counter=");
    Serial.println(wifi_fail_counter);
    return;
  }

  mqtt.loop();
  ArduinoOTA.handle();

  // Перевірка натискання кнопки
  if(btn.click()) {
    // Зміна стану перемикача
    onSwitchCommand(!fanSwitch.getCurrentState(), &fanSwitch);
  }

  // Перевіряємо, чи минув інтервал оновлення
  if(millis() - lastUpdateAt > updateInterval) {
    lastUpdateAt = millis();
    // Моніторинг рівня WiFi сигналу
    int8_t rssi = WiFi.RSSI();
    wifiRssi.setValue(rssi);
  }
}
