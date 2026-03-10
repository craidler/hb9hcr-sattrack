#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <GravityRtc.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "Actuator.h"
#include "Sensor.h"

const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASSWORD";

AsyncWebServer Server(80);
HB9HCR_Actuator Actuator;
HB9HCR_Sensor Sensor;
GravityRtc RTC;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(500);
    }

    if (!LittleFS.begin()) {
        Serial.println("LittleFS failed");
    }

    Serial.println("LitlFS mounted");

    RTC.setup();
    // RTC.adjustRtc(F(__DATE__), F(__TIME__));

    WiFi.softAP(ssid, pass);
    Serial.print("WiFiIP ");
    Serial.println(WiFi.softAPIP());

    delay(1000);
    Sensor.begin();
    Sensor.read();
    Actuator.begin();
    Actuator.home(Sensor.az_degree, Sensor.el_degree);
    delay(500);
    Actuator.move(0, 90);
    Actuator.move(0, -22.5);
    Actuator.move(0, 0);

    Server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    Server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
        Actuator.read();
        Sensor.read();

        String json = "{";
        json += "\"az_deg\":" + String(Sensor.az_degree) + ",";
        json += "\"az_pos\":" + String(Actuator.az_position) + ",";
        json += "\"el_deg\":" + String(Sensor.el_degree) + ",";
        json += "\"el_pos\":" + String(Actuator.el_position) + ",";
        json += "\"eol\":\"1\"}";

        request->send(200, "application/json", json);
    });

    Server.begin();
}

void loop() {
    Actuator.read();
    Sensor.read();

    Serial.printf("az_deg: %.2f° az_offset: %.2f° az_pos: %u el_deg: %.2f° el_offset: %.2f° el_pos: %u\n",
                  Sensor.az_degree,
                  Actuator.az_offset,
                  Actuator.az_position,
                  Sensor.el_degree,
                  Actuator.el_offset,
                  Actuator.el_position);

    // RTC.read();
    // Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n", RTC.year, RTC.month, RTC.day, RTC.hour, RTC.minute, RTC.second);

    delay(1000);
}