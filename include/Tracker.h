#ifndef __HB9HCR_TRACKER__
#define __HB9HCR_TRACKER__

// #include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

class HB9HCR_Tracker {
   private:
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

    State state = State::IDLE;

   public:
    HB9HCR_Tracker() {}

    void begin(AsyncWebServer* Server) {
        // reset config
        Server->on("/reset", HTTP_GET, [this](AsyncWebServerRequest* request) {
            state = State::IDLE;

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
            state = State::EXECUTE;

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
            time(&t);

            data["state"] = toString(state);
            data["countdown"] = cd;
            data["timestamp"] = t;

            serializeJson(data, response);

            request->send(200, "application/json", response);
        });

        // set config
        AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/config", [this](AsyncWebServerRequest* request, JsonVariant& json) {
            state = State::IDLE;

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

        Serial.println("tracker: pass control webhandlers attached");
    }

    void handle() {
        if (State::IDLE == state) {
            return;
        }

        time(&now);

        if (State::EXECUTE == state) {
            if (!valid()) {
                Serial.println("tracker: EXECUTE to IDLE");
                state = State::IDLE;
                return;
            }

            Serial.println("tracker: EXECUTE to STANDBY");
            state = State::STANDBY;
            return;
        }

        if (State::STANDBY == state) {
            if (now >= aos) {
                Serial.println("tracker: STANDBY to TRACK");
                state = State::TRACK;
                cd = 0;
                return;
            }

            cd = aos - now;
            return;
        }

        if (State::TRACK == state) {
            if (now >= los) {
                Serial.println("tracker: TRACK to HOME");
                state = State::HOME;
                cd = 0;
                return;
            }

            cd = los - now;
            return;
        }

        if (State::HOME == state) {
            Serial.println("tracker: HOME to IDLE");
            state = State::IDLE;
            return;
        }
    }

    bool valid() {
        time(&t);
        if (t > aos + 10) return false;      // check for 10 seconds lead
        if (aos + 60 >= los) return false;   // check for 60 seconds minimum pass duration
        if (aos_az == los_az) return false;  // check for different aos and los az
        return true;
    }

    char* toString(HB9HCR_Tracker::State s) {
        switch (s) {
            case HB9HCR_Tracker::State::IDLE:
                return "IDLE";
            case HB9HCR_Tracker::State::EXECUTE:
                return "EXECUTE";
            case HB9HCR_Tracker::State::TRACK:
                return "TRACK";
            case HB9HCR_Tracker::State::HOME:
                return "HOME";
            default:
                return "UNKNOWN";
        }
    }
};

#endif