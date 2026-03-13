#ifndef __HB9HCR_TRACKER__
#define __HB9HCR_TRACKER__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <string.h>

#include "Actuator.h"
#include "Config.h"

class HB9HCR_Tracker {
   private:
    HB9HCR_Actuator* Actuator;
    String response;

    unsigned short aos_az = 0;
    unsigned short aos_el = 0;
    unsigned short los_az = 0;
    unsigned short los_el = 0;
    unsigned short max_el = 0;
    time_t aos = 0;
    time_t max = 0;
    time_t los = 0;
    time_t now = 0;
    time_t cd;
    time_t t;

    enum class State {
        IDLE,
        EXECUTE,
        STANDBY,
        TRACK,
        HOME,
    };

    State _state = State::IDLE;

   public:
    HB9HCR_Tracker(HB9HCR_Actuator* a) : Actuator(a) {}

    void begin(AsyncWebServer* Server) {
        // reset config
        Server->on("/clear", HTTP_GET, [this](AsyncWebServerRequest* request) {
            _state = State::IDLE;

            JsonDocument data;
            time(&t);

            data["aos"] = aos = 0;
            data["aos_az"] = aos_az = 0;
            data["aos_el"] = aos_el = 0;
            data["los"] = los = 0;
            data["los_az"] = los_az = 0;
            data["los_el"] = los_el = 0;
            data["max_el"] = max_el = 0;
            data["timestamp"] = t;

            serializeJson(data, response);

            request->send(200, "application/json", response);
        });

        // execute config
        Server->on("/execute", HTTP_GET, [this](AsyncWebServerRequest* request) {
            _state = State::EXECUTE;

            JsonDocument data;
            time(&t);

            data["aos"] = aos;
            data["aos_az"] = aos_az;
            data["aos_el"] = aos_el;
            data["los"] = los;
            data["los_az"] = los_az;
            data["los_el"] = los_el;
            data["max_el"] = max_el;
            data["timestamp"] = t;

            serializeJson(data, response);

            request->send(200, "application/json", response);
        });

        // state
        Server->on("/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
            JsonDocument data;
            String s;
            time(&t);
            state(&s);

            data["state"] = s;
            data["countdown"] = cd;
            data["timestamp"] = t;

            serializeJson(data, response);

            request->send(200, "application/json", response);
        });

        // set config
        AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/config", [this](AsyncWebServerRequest* request, JsonVariant& json) {
            _state = State::IDLE;

            JsonObject data = json.as<JsonObject>();

            if (data.isNull()) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            time(&t);

            aos = data["aos"];
            aos_az = data["aos_az"];
            aos_el = data["aos_el"];
            los = data["los"];
            los_az = data["los_az"];
            los_el = data["los_el"];
            max_el = data["max_el"];
            data["timestamp"] = t;

            serializeJson(data, response);

            request->send(200, "application/json", response);
        });

        Server->addHandler(configHandler);

        if (HB9HCR_DEBUG) Serial.println("tracker: pass control webhandlers attached");
    }

    void handle() {
        if (State::IDLE == _state) return;

        time(&now);

        if (State::EXECUTE == _state) {
            if (!valid()) {
                if (HB9HCR_DEBUG) Serial.println("tracker: EXECUTE to IDLE");
                _state = State::IDLE;
                return;
            }

            Actuator->moveTo(aos_az, aos_el);
            if (HB9HCR_DEBUG) Serial.println("tracker: EXECUTE to STANDBY");
            _state = State::STANDBY;
            return;
        }

        if (State::STANDBY == _state) {
            if (now >= aos) {
                if (HB9HCR_DEBUG) Serial.println("tracker: STANDBY to TRACK");
                _state = State::TRACK;
                cd = 0;
                return;
            }

            cd = aos - now;
            return;
        }

        if (State::TRACK == _state) {
            if (now >= los) {
                if (HB9HCR_DEBUG) Serial.println("tracker: TRACK to HOME");
                _state = State::HOME;
                cd = 0;
                return;
            }

            float az, el;
            current(&az, &el);
            // Actuator->moveTo(az, el);
            if (HB9HCR_DEBUG) Serial.printf("actuator: move to %.2f°:%.2f°\n", az, el);
            cd = los - now;
            return;
        }

        if (State::HOME == _state) {
            Actuator->moveTo(aos_az, aos_el);
            if (HB9HCR_DEBUG) Serial.println("tracker: HOME to IDLE");
            _state = State::IDLE;
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
        switch (_state) {
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

            case HB9HCR_Tracker::State::HOME:
                *s = "HOME";
                return true;

            default:
                *s = "UNKNOWN";
        }

        return false;
    }
};

#endif