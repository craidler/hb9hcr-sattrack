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
    Actuator.home(0, Sensor.el_degree);
    Actuator.move(45, 45);
    Actuator.move(315, -45);
    Actuator.move(0, 0);

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
        Actuator.read();
        Sensor.read();

        String json = "{";
        json += "\"az_axis\":" + String(Actuator.az_current) + ",";
        json += "\"az_sensor\":" + String(Sensor.az_degree) + ",";
        json += "\"el_axis\":" + String(Actuator.el_current) + ",";
        json += "\"el_sensor\":" + String(Sensor.el_degree) + ",";
        json += "\"eol\":\"1\"}";

        request->send(200, "application/json", json);
    });

    Server.on("/move", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("axis") || !request->hasParam("target")) return;
        if (0 == request->getParam("axis")->value().compareTo("az")) Actuator.moveAz(request->getParam("target")->value().toFloat());
        if (0 == request->getParam("axis")->value().compareTo("el")) Actuator.moveEl(request->getParam("target")->value().toFloat());
        request->send(200, "application/json" "{}");
    });

    Server.begin();
}

void loop() {
    Actuator.read();
    Sensor.read();

    Serial.printf("az pos: %d el pos: %d\n", Actuator.az_position, Actuator.el_position);

    /*
    Serial.printf("az_deg: %.2f° az_offset: %.2f° az_pos: %.2f° el_deg: %.2f° el_offset: %.2f° el_pos: %.2f°\n",
                  Sensor.az_degree,
                  Actuator.az_offset,
                  Actuator.az_current,
                  Sensor.el_degree,
                  Actuator.el_offset,
                  Actuator.el_current);
                  */

    // RTC.read();
    // Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n", RTC.year, RTC.month, RTC.day, RTC.hour, RTC.minute, RTC.second);

    delay(1000);
}