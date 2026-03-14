#ifndef __HB9HCR_TRACKER__
#define __HB9HCR_TRACKER__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

#include "Actuator.h"

class HB9HCR_Tracker {
   private:
    unsigned short aos_az, aos_el, los_az, los_el, max_el;
    JsonDocument data;
    String response;
    time_t aos, max, los, now, cd, t;

   public:
    enum class State {
        IDLE,
        EXECUTE,
        STANDBY,
        TRACK,
        PARK,
    };

    HB9HCR_Actuator* Actuator = nullptr;
    AsyncWebServer* Server = nullptr;
    State State = State::IDLE;

    HB9HCR_Tracker() {}

    void begin() {
        if (Server != nullptr) {
            Serial.println("tracker: pass control webhandlers ");
            // reset config
            Server->on("/clear", HTTP_GET, [this](AsyncWebServerRequest* request) {
                State = State::IDLE;
                data.clear();
                time(&t);

                data["aos"] = aos = 0;
                data["aos_az"] = aos_az = 0;
                data["aos_el"] = aos_el = 0;
                data["los"] = los = 0;
                data["los_az"] = los_az = 0;
                data["los_el"] = los_el = 0;
                data["max_el"] = max_el = 0;
                data["state"] = "";
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // execute config
            Server->on("/execute", HTTP_GET, [this](AsyncWebServerRequest* request) {
                State = State::EXECUTE;
                data.clear();
                time(&t);

                data["aos"] = aos;
                data["aos_az"] = aos_az;
                data["aos_el"] = aos_el;
                data["los"] = los;
                data["los_az"] = los_az;
                data["los_el"] = los_el;
                data["max_el"] = max_el;
                data["state"] = "";
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // state
            Server->on("/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
                String s;
                data.clear();
                time(&t);
                state(&s);

                data["aos"] = aos;
                data["aos_az"] = aos_az;
                data["aos_el"] = aos_el;
                data["los"] = los;
                data["los_az"] = los_az;
                data["los_el"] = los_el;
                data["max_el"] = max_el;
                data["state"] = "";
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // set config
            AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/config", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                State = State::IDLE;
                data = json.as<JsonObject>();

                if (data.isNull()) {
                    data.clear();
                    data["error"] = "invalid json";
                    serializeJson(data, response);
                    request->send(400, "application/json", response);
                    return;
                }

                time(&t);
                aos = data["aos"];
                los = data["los"];
                aos_az = data["aos_az"];
                aos_el = data["aos_el"];
                los_az = data["los_az"];
                los_el = data["los_el"];
                max_el = data["max_el"];
                data["state"] = "";
                data["timestamp"] = t;
                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            Server->addHandler(configHandler);
        }

        Serial.println("attached");
    }

    void loop() {
        if (State::IDLE == State) return;

        time(&now);

        if (State::EXECUTE == State) {
            if (!valid()) {
                Serial.println("tracker: EXECUTE to IDLE");
                State = State::IDLE;
                return;
            }

            if (Actuator != nullptr) Actuator->moveTo(aos_az, aos_el);
            Serial.println("tracker: EXECUTE to STANDBY");
            State = State::STANDBY;
            return;
        }

        if (State::STANDBY == State) {
            if (now >= aos) {
                Serial.println("tracker: STANDBY to TRACK");
                State = State::TRACK;
                cd = 0;
                return;
            }

            cd = aos - now;
            return;
        }

        if (State::TRACK == State) {
            if (now >= los) {
                Serial.println("tracker: TRACK to PARK");
                State = State::PARK;
                cd = 0;
                return;
            }

            float az, el;
            current(&az, &el);
            if (Actuator != nullptr) Actuator->moveTo(az, el);
            Serial.printf("actuator: move to %.2f°:%.2f°\n", az, el);
            cd = los - now;
            return;
        }

        if (State::PARK == State) {
            if (Actuator != nullptr) Actuator->moveTo(aos_az, aos_el);
            Serial.println("tracker: PARK to IDLE");
            State = State::IDLE;
            return;
        }
    }

    bool current(float* az, float* el) {
        // azimuth follows a linear path
        unsigned short delta = (los_az - aos_az) % 360;
        if (delta > 180) delta -= 360.0f;
        *az = ((int)(aos_az + delta * progress() * 100) % 36000) / 100;
        // elevation follows a sinusoidal path

        return true;
    }

    bool valid() {
        time(&t);
        if (t > aos + 10) return false;      // check for 10 seconds lead
        if (aos + 60 >= los) return false;   // check for 60 seconds minimum pass duration
        if (aos_az == los_az) return false;  // check for different aos and los az
        return true;
    }

    float progress() {
        time(&t);
        unsigned int duration = los - aos;
        unsigned int progress = t - aos;
        return progress / duration;
    }

    bool state(String* s) {
        switch (State) {
            case HB9HCR_Tracker::State::IDLE:
                *s = "IDLE";
                return true;

            case HB9HCR_Tracker::State::EXECUTE:
                *s = "EXECUTE";
                return true;

            case HB9HCR_Tracker::State::STANDBY:
                *s = "STANDBY";
                return true;

            case HB9HCR_Tracker::State::TRACK:
                *s = "TRACK";
                return true;

            case HB9HCR_Tracker::State::PARK:
                *s = "PARK";
                return true;

            default:
                *s = "UNKNOWN";
        }

        return false;
    }
};

#endif