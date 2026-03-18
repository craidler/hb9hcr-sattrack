#ifndef __HB9HCR_TRACKER__
#define __HB9HCR_TRACKER__

#include <AsyncJson.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

#include "Actuator.h"

class HB9HCR_Tracker {
   private:
    struct TrackingPoint {
        unsigned long time = 0;
        unsigned short az = 0;
        unsigned short el = 0;
    };

    TrackingPoint aos, mel, los;
    JsonDocument data;
    String response;
    long cd;

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
    ESP32Time* Clock = nullptr;
    State State = State::IDLE;

    HB9HCR_Tracker() {}

    void begin() {
        if (Server != nullptr) {
            Serial.print("tracker : pass control webhandlers ");

            // state
            Server->on("/tracker/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // config
            AsyncCallbackJsonWebHandler* handleSet = new AsyncCallbackJsonWebHandler("/tracker/set", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                State = State::IDLE;
                data = json.as<JsonObject>();

                aos.time = data["aos_time"].as<long>();
                aos.az = data["aos_az"].as<short>();
                aos.el = data["aos_el"].as<short>();

                los.time = data["los_time"].as<long>();
                los.az = data["los_az"].as<short>();
                los.el = data["los_el"].as<short>();

                mel.time = aos.time + (los.time - aos.time) / 2;
                mel.az = 0;
                mel.el = data["mel_el"].as<short>();

                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            handleSet->setMethod(HTTP_POST);
            Server->addHandler(handleSet);

            // execute
            AsyncCallbackJsonWebHandler* handleExecute = new AsyncCallbackJsonWebHandler("/tracker/execute", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                State = State::EXECUTE;
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            handleExecute->setMethod(HTTP_POST);
            Server->addHandler(handleExecute);

            // reset
            Server->on("/tracker/reset", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
                State = State::IDLE;
                aos.time = aos.az = aos.el = 0;
                mel.time = mel.az = mel.el = 0;
                los.time = los.az = los.el = 0;
                cd = 0;

                // Use a local String or the existing response buffer
                String response;
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            Serial.println("attached");
        }
    }

    // tracker state machine
    void loop() {
        // idling ...
        if (State::IDLE == State) return;

        // exe button was pressed, validate configuration, if fine move to initial position and pass on to standby
        if (State::EXECUTE == State) {
            if (!valid()) {
                Serial.println("tracker: EXECUTE to IDLE");
                State = State::IDLE;
                return;
            }

            if (Actuator != nullptr) {
                Actuator->Azimuth.to(aos.az);
                Actuator->Elevation.to(aos.el);
            }

            Serial.println("tracker: EXECUTE to STANDBY");
            State = State::STANDBY;
            return;
        }

        // wait until aos and pass on to track
        if (State::STANDBY == State) {
            if (Clock->getEpoch() >= aos.time) {
                Serial.println("tracker: STANDBY to TRACK");
                State = State::TRACK;
                cd = 0;
                return;
            }

            cd = aos.time - Clock->getEpoch();
            return;
        }

        // track satellite pass, if los pass to park
        if (State::TRACK == State) {
            if (Clock->getEpoch() >= los.time) {
                Serial.println("tracker: TRACK to PARK");
                State = State::PARK;
                cd = 0;
                return;
            }

            TrackingPoint cur;
            current(&cur);

            if (Actuator != nullptr) {
                Actuator->Azimuth.to(cur.az);
                Actuator->Elevation.to(cur.el);
            }

            cd = los.time - cur.time;
            return;
        }

        // park actuator and pass on to idle
        if (State::PARK == State) {
            if (Actuator != nullptr) {
                Actuator->Azimuth.to(aos.az);
                Actuator->Elevation.to(aos.el);
            }

            Serial.println("tracker: PARK to IDLE");
            State = State::IDLE;
            return;
        }
    }

    // calculate the current target angles for both axis based on progress
    void current(TrackingPoint* cur) {
        cur->time = Clock->getEpoch();
        float p = progress();

        // linear azimuth
        float d = los.az - aos.az;
        if (d > 180.0) d -= 360.0;
        if (d < -180.0) d += 360.0;
        cur->az = aos.az + (d * p);
        cur->az = fmod(cur->az, 360.0);
        if (cur->az < 0) cur->az += 360.0;

        // elevation sinusoidal
        cur->el = (mel.el - aos.el) * sin(M_PI * p) + aos.el;
    }

    // validate pass configuration
    bool valid() {
        if (Clock->getEpoch() > aos.time + 10) return false;  // check for 10 seconds lead
        if (aos.time + 60 == los.time) return false;          // check for 60 seconds minimum pass duration
        if (aos.az == los.az) return false;                   // check for different aos and los az
        if (aos.el == mel.el) return false;                   // aos elevation like mel does not make sense
        if (los.el == mel.el) return false;                   // los elevation like mel does not make sense
        return true;
    }

    // calculate progress based on passed time
    float progress() {
        long d = los.time - aos.time;
        float p = Clock->getEpoch() - aos.time;
        return p / d;
    }

    JsonDocument* getJson() {
        data.clear();

        data["aos_time"] = aos.time;
        data["aos_az"] = aos.az;
        data["aos_el"] = aos.el;

        data["los_time"] = los.time;
        data["los_az"] = los.az;
        data["los_el"] = los.el;

        data["mel_time"] = mel.time;
        data["mel_az"] = mel.az;
        data["mel_el"] = mel.el;

        data["state"] = stateToString();
        data["cd"] = cd;

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