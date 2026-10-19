#include "LittleFS.h"
#include <web_interface.h>
#include <ESPAsyncWebServer.h>

#include <mqtt.h>

String escapeHtml(const String &value) {
  String escaped;
  escaped.reserve(value.length());
  for (size_t i = 0; i < value.length(); i++) {
    switch (value[i]) {
    case '&':
      escaped += F("&amp;");
      break;
    case '<':
      escaped += F("&lt;");
      break;
    case '>':
      escaped += F("&gt;");
      break;
    case '"':
      escaped += F("&quot;");
      break;
    case '\'':
      escaped += F("&#39;");
      break;
    default:
      escaped += value[i];
    }
  }
  return escaped;
}

void handleHomePage(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/index.html", String());
}

void handleRunZone(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/runzone.html", String());
}

void handleRunProgram(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/runprogram.html", String());
}

void handleMqttSettingsPage(AsyncWebServerRequest *request) {
  String page;
  page.reserve(1800);
  page += F("<!DOCTYPE html><html lang=\"en\"><head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
            "<title>MQTT Settings</title><style>"
            "body{font:16px sans-serif;margin:2rem auto;max-width:34rem;padding:0 1rem}"
            "label{display:block;margin-top:1rem}"
            "input[type=text],input[type=number],input[type=password]"
            "{box-sizing:border-box;padding:.5rem;width:100%}"
            "button{margin-top:1rem;padding:.6rem 1rem}"
            "</style></head><body><p><a href=\"/home\">Home</a></p>"
            "<h1>MQTT Settings</h1><form method=\"POST\" action=\"/settings\">"
            "<label for=\"mqtt_server\">Broker address</label>"
            "<input id=\"mqtt_server\" name=\"mqtt_server\" type=\"text\" maxlength=\"253\" value=\"");
  page += escapeHtml(MqttSettings.server);
  page += F("\" required><label for=\"mqtt_port\">Port</label>"
            "<input id=\"mqtt_port\" name=\"mqtt_port\" type=\"number\" min=\"1\" max=\"65535\" value=\"");
  page += String(MqttSettings.port > 0 && MqttSettings.port <= 65535
                     ? MqttSettings.port
                     : 1883);
  page += F("\" required><label for=\"mqtt_username\">Username</label>"
            "<input id=\"mqtt_username\" name=\"mqtt_username\" type=\"text\" maxlength=\"99\" value=\"");
  page += escapeHtml(MqttSettings.username);
  page += F("\"><label for=\"mqtt_password\">New password</label>"
            "<input id=\"mqtt_password\" name=\"mqtt_password\" type=\"password\" maxlength=\"99\" "
            "autocomplete=\"new-password\"><p>Leave blank to keep the current password.</p>"
            "<label><input name=\"clear_password\" type=\"checkbox\" value=\"1\"> "
            "Clear saved password</label><label><input name=\"mqtt_enabled\" type=\"checkbox\" "
            "value=\"1\"");
  if (MqttSettings.enable) {
    page += F(" checked");
  }
  page += F("> Enable MQTT</label><button type=\"submit\">Save and reconnect</button></form>"
            "<p>Settings are saved on the device. MQTT credentials are submitted over HTTP; "
            "use this page only on a trusted local network.</p></body></html>");
  request->send(200, "text/html", page);
}