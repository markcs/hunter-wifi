/**
 * First version: July 2020 - Eloi Codina Torras
 */
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>

#include <mqtt.h>
#include <web_server.h>
#include <wifi.h>

AsyncWebServer server = AsyncWebServer(80);
bool wifiDisconnected = false;
bool enableMQTT = true;
MQTT MqttSettings;
MQTT from_wifi_manager;

bool updateMqttSettings(const MQTT &settings) {
  DynamicJsonDocument doc(1024);
  doc["mqtt_server"] = settings.server;
  doc["mqtt_port"] = settings.port;
  doc["mqtt_username"] = settings.username;
  doc["mqtt_password"] = settings.password;
  doc["mqtt_enable"] = settings.enable;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
    return false;
  }

  size_t bytesWritten = serializeJsonPretty(doc, configFile);
  configFile.close();
  if (bytesWritten == 0) {
    Serial.println("failed to write MQTT config");
    return false;
  }

  MqttSettings = settings;
  client.disconnect();
  if (MqttSettings.enable) {
    client.setServer(MqttSettings.server.c_str(), MqttSettings.port);
    client.setCallback(mqtt_callback);
  }
  Serial.printf("MQTT settings saved: %s:%d (enabled: %s)\n",
                MqttSettings.server.c_str(), MqttSettings.port,
                MqttSettings.enable ? "yes" : "no");
  return true;
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  // WiFi.mode(WIFI_STA);
  Serial.println("mounting FS...");
  if (LittleFS.begin()) {
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, configFile);
        serializeJson(doc, Serial);
        if (error) {
          Serial.print(F("\ndeserializeJson() failed: "));
          Serial.println(error.c_str());
          Serial.println("failed to load json config");
        } else {
          Serial.println("\njson successfully parsed");
          MqttSettings.server = doc["mqtt_server"].as<String>();
          MqttSettings.port = doc["mqtt_port"];
          MqttSettings.username = doc["mqtt_username"].as<String>();
          MqttSettings.password = doc["mqtt_password"].as<String>();
          MqttSettings.enable = doc["mqtt_enable"].as<bool>();
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  // this is here just to make sure that if wifimanager is
  // skipped when config is saved, MQTT still works, otherwise empty data would
  // override what we learned from config file saved on disk
  from_wifi_manager = setup_wifi();
  if (from_wifi_manager.server != "") {
    MqttSettings = from_wifi_manager;
  }
  if (MqttSettings.port < 1 || MqttSettings.port > 65535) {
    MqttSettings.port = 1883;
  }
  setup_WebServer();
  if (shouldSaveConfig) {
    Serial.println("saving config");
    MqttSettings.enable = true;
    updateMqttSettings(MqttSettings);
  }
  if (MqttSettings.enable) {
    client.setServer(MqttSettings.server.c_str(), MqttSettings.port);
    client.setCallback(mqtt_callback);
  }
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("%s:%d\n", MqttSettings.server.c_str(), MqttSettings.port);
  Serial.printf("Ready! Open http://%s.local in your browser\n", HOSTNAME);
}

void loop() {
  static bool mqttAttempted = false;
  static unsigned long lastMqttAttempt = 0;
  const unsigned long mqttRetryInterval = 5000;

  if (MqttSettings.enable && !client.connected() &&
      (!mqttAttempted || millis() - lastMqttAttempt >= mqttRetryInterval)) {
    mqttAttempted = true;
    lastMqttAttempt = millis();
    mqtt_connect(MqttSettings.username.c_str(),
                 MqttSettings.password.c_str());
    if (!client.connected()) {
      Serial.printf("MQTT connection failed (state %d); retrying in 5 seconds\n",
                    client.state());
    }
  }
  // wifiDisconnected = checkWifiConnection(wifiDisconnected);
  client.loop();
  MDNS.update();
  // timer.run();
}