#include <Arduino.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "Actuator.h"
#include "Sensor.h"
#include "Tracker.h"

const char* ssid = "HB9HCR-Sattrack";
const char* pass = "Sattrack-2026!";

AsyncWebServer Server(80);
HB9HCR_Actuator Actuator;
HB9HCR_Tracker Tracker;
HB9HCR_Sensor Sensor;
ESP32Time Clock;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(500);

    Sensor.begin();

    Actuator.Input = &Serial;
    Actuator.Sensor = &Sensor;
    Actuator.Server = &Server;
    Actuator.begin();

    Tracker.Actuator = &Actuator;
    Tracker.Server = &Server;
    Tracker.Clock = &Clock;
    Tracker.begin();

    Serial.print("littlefs: ");

    if (!LittleFS.begin()) {
        Serial.println("failed");
        return;
    }

    Serial.println("mounted");

    Serial.print("wifi    : access point ");
    WiFi.softAP(ssid, pass);
    Serial.println(WiFi.softAPIP());

    Server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    Server.on("/ui.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/ui.css", "text/css");
    });

    Server.on("/ui.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/ui.js", "text/javascript");
    });

    Server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/favicon.svg", "image/svg+xml");
    });

    Server.on("/symbol.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/symbol.svg", "image/svg+xml");
    });

    // set time
    AsyncCallbackJsonWebHandler* setTime = new AsyncCallbackJsonWebHandler("/time", [](AsyncWebServerRequest* request, JsonVariant& json) {
        JsonObject data = json.as<JsonObject>();
        String response;
        Clock.setTime(data["value"].as<long>());
        data.clear();
        data["time"] = Clock.getEpoch();

        serializeJson(data, response);
        request->send(200, "application/json", response);
    });

    setTime->setMethod(HTTP_POST);
    Server.addHandler(setTime);

    Server.begin();
}

void loop() {
    Actuator.loop();
    Tracker.loop();
}