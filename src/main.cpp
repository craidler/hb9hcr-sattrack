#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <time.h>

#include "Actuator.h"
#include "Tracker.h"
#include "Sensor.h"
#include "Clock.h"

const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASSWORD";
char datetime[20];

AsyncWebServer Server(80);
HB9HCR_Actuator Actuator;
HB9HCR_Tracker Tracker;
HB9HCR_Sensor Sensor;
HB9HCR_Clock Clock;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(500);
    }

    if (!LittleFS.begin()) {
        Serial.println("littlefs: failed");
    }

    Serial.println("littlefs: mounted");

    WiFi.softAP(ssid, pass);
    Serial.print("wifi: access point at ");
    Serial.println(WiFi.softAPIP());

    Clock.begin();

    Sensor.begin();
    Actuator.begin(&Server);

    Sensor.calibrate(&Actuator);
   

    Server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    Server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/favicon.svg", "image/svg+xml");
    });

    Server.on("/symbol.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/symbol.svg", "image/svg+xml");
    });

    Server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
        String dt;
        time_t t;
        Clock.datetime(dt);
        time(&t);
        Sensor.read();

        String json = "{";
        json += "\"az_axis\":" + String(Actuator.degree[0]) + ",";
        json += "\"az_sensor\":" + String(Sensor.degree[0]) + ",";
        json += "\"el_axis\":" + String(Actuator.degree[1]) + ",";
        json += "\"el_sensor\":" + String(Sensor.degree[1]) + ",";
        json += "\"datetime\":\"" + String(dt.c_str()) + "\"}";
        json += "\"timestamp\":\"" + String(t) + "\"}";
        request->send(200, "application/json", json);

        Serial.println(json);
    });

    Tracker.begin(&Server);

    Server.begin();
}

void loop() {
    Tracker.handle();
    delay(1000);
}