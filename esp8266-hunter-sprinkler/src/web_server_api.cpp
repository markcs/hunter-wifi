/**
 * This file includes all the functions that are called by the webserver to
 * serve API paths. It makes it easier to control the actions to be performed to
 * the Hunter Irrigation System.
 *
 * First version: July 2020 - Eloi Codina Torras
 */
#include <Schedule.h>

#include <mqtt.h>
#include <web_server_api.h>
#include <web_server_scheduled.h>

void saveMqttSettingsRequest(AsyncWebServerRequest *request) {
  if (!request->hasParam("mqtt_server", true) ||
      !request->hasParam("mqtt_port", true) ||
      !request->hasParam("mqtt_username", true) ||
      !request->hasParam("mqtt_password", true)) {
    request->send(400, "text/plain", "Missing MQTT settings.");
    return;
  }

  MQTT settings = MqttSettings;
  settings.server = request->getParam("mqtt_server", true)->value();
  settings.server.trim();
  String portText = request->getParam("mqtt_port", true)->value();
  settings.username = request->getParam("mqtt_username", true)->value();
  String password = request->getParam("mqtt_password", true)->value();
  bool clearPassword = request->hasParam("clear_password", true);
  settings.enable = request->hasParam("mqtt_enabled", true);

  bool validPort = portText.length() > 0 && portText.length() <= 5;
  for (size_t i = 0; validPort && i < portText.length(); i++) {
    validPort = portText[i] >= '0' && portText[i] <= '9';
  }
  long port = validPort ? portText.toInt() : 0;

  if (settings.server.length() == 0 || settings.server.length() > 253 ||
      settings.username.length() > 99 || password.length() > 99 ||
      port < 1 || port > 65535) {
    request->send(400, "text/plain", "Invalid MQTT server, port, or credentials.");
    return;
  }
  for (size_t i = 0; i < settings.server.length(); i++) {
    if (settings.server[i] <= ' ') {
      request->send(400, "text/plain", "MQTT server must not contain whitespace.");
      return;
    }
  }

  settings.port = static_cast<int>(port);
  if (clearPassword) {
    settings.password = "";
  } else if (password.length() > 0) {
    settings.password = password;
  }

  bool queued = schedule_function([settings]() {
    updateMqttSettings(settings);
  });
  if (!queued) {
    request->send(503, "text/plain", "Device is busy; please try again.");
    return;
  }
  request->send(202, "text/plain",
                "Settings update queued. Wait a few seconds, then reload /settings.");
}

/**
 * Extract the latest parameter on the URL given the request and the base path.
 * It has to be done with this "hack" as REGEX doesnt't work properly on the
 * ESP8266 currently.
 *
 * Example:
 * AsyncWebServerRequest request </api/start/zone/3>
 * int id = getIdFromURL(request, "/api/start/zone/");
 * Serial.println(id);
 *  --> 3
 *
 * @param request request made by the client
 * @param root base path of the URL.
 */
int getIdFromURL(AsyncWebServerRequest *request, String root) {
  String string_id = request->url();
  string_id.replace(root, "");
  int id = string_id.toInt();
  return id;
}

/**
 * Function to be called when the client makes a request to
 * /api/start/zone
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to start the zone with the HunterRoam library.
 *
 * @param request request made by the client
 */
void startZoneRequest(AsyncWebServerRequest *request) {
  if (!(request->hasParam("time"))) {
    request->send_P(400, "text/plain", "Please include parameter 'time'.");
  } else {
    byte num = getIdFromURL(request, "/api/start/zone/");
    byte time = request->getParam("time")->value().toInt();

    String result;
    String toReturn;
    if (!(request->hasParam("result"))) {
      result = "";
      toReturn = "\"As you have not specified a 'result' argument, the result "
                 "of the operation can't be send to you.\"";
    } else {
      result = (String)request->getParam("result")->value();
      toReturn = "\"After the action, the result will be sent to the webhook "
                 "url you have specified.\"";
    }

    schedule_function([num, time, result]() { startZone(num, time, result); });

    request->send_P(200, "application/json",
                    ((String) "{\"status\": \"Request received and processed "
                              "correctly.\"," +
                     (String) "\"result\": " + toReturn + "}")
                        .c_str());
  }
}

/**
 * Function to be called when the client makes a request to
 * /api/start/program
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to start the program with the HunterRoam library.
 *
 * @param request request made by the client
 */
void startProgramRequest(AsyncWebServerRequest *request) {

  String result;
  String toReturn;
  if (!(request->hasParam("result"))) {
    result = "";
    toReturn = "\"As you have not specified a 'result' argument, the result of "
               "the operation can't be send to you.\"";
  } else {
    result = request->getParam("result")->value();
    toReturn = "\"After the action, the result will be sent to the webhook url "
               "you have specified.\"";
  }

  byte num = getIdFromURL(request, "/api/start/program/");
  schedule_function([num, result]() { startProgram(num, result); });

  request->send_P(
      200, "application/json",
      ((String) "{\"status\": \"Request received and processed correctly.\"," +
       (String) "\"result\": " + toReturn + "}")
          .c_str());
}

/**
 * Function to be called when the client makes a request to
 * /api/stop/zone
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to stop the zone with the HunterRoam library.
 *
 * @param request request made by the client
 */
void stopZoneRequest(AsyncWebServerRequest *request) {

  String result;
  String toReturn;
  if (!(request->hasParam("result"))) {
    result = "";
    toReturn = "\"As you have not specified a 'result' argument, the result of "
               "the operation can't be send to you.\"";
  } else {
    result = request->getParam("result")->value();
    toReturn = "\"After the action, the result will be sent to the webhook url "
               "you have specified.\"";
  }

  byte num = getIdFromURL(request, "/api/stop/zone/");
  schedule_function([num, result]() { stopZone(num, result); });

  request->send_P(
      200, "application/json",
      ((String) "{\"status\": \"Request received and processed correctly.\"," +
       (String) "\"result\": " + toReturn + "}")
          .c_str());
}