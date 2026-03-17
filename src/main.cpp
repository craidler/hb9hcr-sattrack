#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "Actuator.h"
#include "Tracker.h"
#include "Sensor.h"
#include "Clock.h"

const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASSWORD";

AsyncWebServer Server(80);
HB9HCR_Actuator Actuator;
HB9HCR_Tracker Tracker;
HB9HCR_Sensor Sensor;
HB9HCR_Clock Clock;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(500);

    Clock.begin();
    Sensor.begin();

    Actuator.Clock = &Clock;
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

    Server.on("/ui.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/ui.css", "text/css");
    });

    Server.on("/ui.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/ui.js", "text/javascript");
    });

    Server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    Server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/favicon.svg", "image/svg+xml");
    });

    Server.on("/symbol.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/symbol.svg", "image/svg+xml");
    });

    Server.begin();
}

void loop() {
    Actuator.loop();
    Tracker.loop();
    delay(100);
}