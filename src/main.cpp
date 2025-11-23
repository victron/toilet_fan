#include <Arduino.h>
#include <ArduinoHA.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "OTAHandler.h"
#include "button.h"
#include "secrets.h"

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
// button btn(BUTTON_PIN);

bool setupWiFi(unsigned long retry_interval = 30000) {
  static unsigned long lastWiFiAttempt = 0;
  // якщо вже підключені — ок
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  // перша спроба має відбутися одразу (lastWiFiAttempt == 0) або по таймауту
  if (lastWiFiAttempt == 0 || millis() - lastWiFiAttempt >= retry_interval) {
    lastWiFiAttempt = millis();
    Serial.print("connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);                 // гарантуємо режим станції
    WiFi.setAutoReconnect(true);         // авто перепідключення
    WiFi.hostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, ((millis() / 200) % 2) ? HIGH : LOW);
  } else {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  }
  return (WiFi.status() == WL_CONNECTED);
}

void onSwitchCommand(bool state, HASwitch* sender) {
  // SSR only, instead MSR
  // digitalWrite(SSR_PIN, (state ? HIGH : LOW));
  digitalWrite(SSR_PIN, (state ? LOW : HIGH));  // інвертна логіка
  // ssr підключений "-" до вивода, інший на +3.3В
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

  // одразу встановити режим і спробувати під'єднатися
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  setupWiFi();

  // Логування розміру флеш-пам'яті
  // uint32_t flashSize = ESP.getFlashChipRealSize();
  // Serial.print("Flash size: ");
  // Serial.print(flashSize);
  // Serial.println(" bytes");

  // set device's details (optional)
  device.setName(HOSTNAME);
  device.setSoftwareVersion("1.0.0");

  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, HIGH);
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
  // Перевірка натискання кнопки
  // if(btn.click()) {
  //   // Зміна стану перемикача
  //   // TODO: in esp-01 on pin 3 (RX) there is a problem. switch flashing permanently.
  //   // робити тригер щоб Serial.end(); через якийсь час.
  //   // продумати, щоб можна було підключити serial до цього часу.
  //   // onSwitchCommand(!fanSwitch.getCurrentState(), &fanSwitch);
  // }

  // ------------ remote -----------------------
  if(!connected && WiFi.status() == WL_CONNECTED) {
    // not mqtt and connected to wifi
    Serial.println("WiFi OK, mqtt NOK");
    digitalWrite(LED, (millis() / 1000) % 2);
  }

  if(!setupWiFi()) return;

  mqtt.loop();
  ArduinoOTA.handle();

  // Перевіряємо, чи минув інтервал оновлення
  if(millis() - lastUpdateAt > 5000) {
    lastUpdateAt = millis();
    // Моніторинг рівня WiFi сигналу
    wifiRssi.setValue(WiFi.RSSI());
  }
}