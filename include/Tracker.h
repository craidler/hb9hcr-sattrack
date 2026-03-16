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
    time_t aos, max, los, cd, ts;

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
            Serial.print("tracker : pass control webhandlers ");

            // reset
            Server->on("/tracker", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
                State = State::IDLE;
                aos = 0;
                los = 0;
                aos_az = 0;
                aos_el = 0;
                los_az = 0;
                los_el = 0;
                max_el = 0;

                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // state
            Server->on("/tracker", HTTP_GET, [this](AsyncWebServerRequest* request) {
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // config
            AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/tracker", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                if ("PATCH" != request->methodToString()) return;

                State = State::IDLE;
                data = json.as<JsonObject>();

                if (data.isNull()) {
                    data.clear();
                    data["error"] = "invalid json";
                    serializeJson(data, response);
                    request->send(400, "application/json", response);
                    return;
                }

                aos = data["aos"];
                los = data["los"];
                aos_az = data["aos_az"];
                aos_el = data["aos_el"];
                los_az = data["los_az"];
                los_el = data["los_el"];
                max_el = data["max_el"];

                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // execute
            AsyncCallbackJsonWebHandler* executeHandler = new AsyncCallbackJsonWebHandler("/tracker", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                if ("POST" != request->methodToString()) return;
                State = State::EXECUTE;
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            Server->addHandler(configHandler);
        }

        Serial.println("attached");
    }

    // tracker state machine
    void loop() {
        // idling ...
        if (State::IDLE == State) return;

        time(&ts);

        // exe button was pressed, validate configuration, if fine move to initial position and pass on to standby
        if (State::EXECUTE == State) {
            if (!valid()) {
                Serial.println("tracker: EXECUTE to IDLE");
                State = State::IDLE;
                return;
            }

            if (Actuator != nullptr) {
                Actuator->Azimuth.to(aos_az);
                Actuator->Elevation.to(aos_el);
            }
            
            Serial.println("tracker: EXECUTE to STANDBY");
            State = State::STANDBY;
            return;
        }

        // wait until aos and pass on to track
        if (State::STANDBY == State) {
            if (ts >= aos) {
                Serial.println("tracker: STANDBY to TRACK");
                State = State::TRACK;
                cd = 0;
                return;
            }

            cd = aos - ts;
            return;
        }

        // track satellite pass, if los pass to park
        if (State::TRACK == State) {
            if (ts >= los) {
                Serial.println("tracker: TRACK to PARK");
                State = State::PARK;
                cd = 0;
                return;
            }

            float az, el;
            current(&az, &el);
            
            if (Actuator != nullptr) {
                Actuator->Azimuth.to(az);
                Actuator->Elevation.to(el);
            }

            Serial.printf("actuator: move to %.2f°:%.2f°\n", az, el);
            cd = los - ts;
            return;
        }

        // park actuator and pass on to idle
        if (State::PARK == State) {
            if (Actuator != nullptr) {
                Actuator->Azimuth.to(aos_az);
                Actuator->Elevation.to(aos_el);
            }

            Serial.println("tracker: PARK to IDLE");
            State = State::IDLE;
            return;
        }
    }

    // calculate the current target angles for both axis based on progress
    bool current(float* az, float* el) {
        return true;
    }

    // validate pass configuration
    bool valid() {
        time(&ts);
        if (ts > aos + 10) return false;      // check for 10 seconds lead
        if (aos + 60 >= los) return false;   // check for 60 seconds minimum pass duration
        if (aos_az == los_az) return false;  // check for different aos and los az
        return true;
    }

    // calculate progress based on passed time
    float progress() {
        // TODO: prevent division by zero
        time(&ts);
        unsigned int duration = los - aos;
        unsigned int progress = ts - aos;
        return progress / duration;
    }

    JsonDocument* getJson() {
        data.clear();
        time(&ts);

        data["aos"] = aos;
        data["los"] = los;
        data["aos_az"] = aos_az;
        data["aos_el"] = aos_el;
        data["los_az"] = los_az;
        data["los_el"] = los_el;
        data["max_el"] = max_el;
        data["state"] = stateToString();
        data["cd"] = cd;
        data["ts"] = ts;

        return &data;
    }

    // return the state as a string
    const char* stateToString() {
        switch (State) {
            case HB9HCR_Tracker::State::IDLE:
                return "IDLE";

            case HB9HCR_Tracker::State::EXECUTE:
                return "EXECUTE";

            case HB9HCR_Tracker::State::STANDBY:
                return "STANDBY";

            case HB9HCR_Tracker::State::TRACK:
                return "TRACK";

            case HB9HCR_Tracker::State::PARK:
                return "PARK";

            default:
                return "UNKNOWN";
        }
    }
};

#endif