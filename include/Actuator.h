#ifndef __HB9HCR_ACTUATOR__
#define __HB9HCR_ACTUATOR__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SCServo.h>
#include <string.h>

#include "Sensor.h"

class HB9HCR_Actuator {
   private:
    AsyncWebServer* _Server;
    HB9HCR_Sensor* _Sensor;
    USBCDC* _Serial;

    String _command;
    String response;
    SMS_STS Servo;
    time_t t;
    char _c;

   public:
    static constexpr float STP_DEG = 4096.0f / 360.0f;  // steps per degree
    static constexpr int STP_TRN = 4096;                // steps per full turn
    static constexpr int AZ_MAX = 3068;                 // cable tangling prevention
    float degree[2] = {0};
    long position[2] = {0};

    HB9HCR_Actuator() {}

    void bind(AsyncWebServer* Server) {
        _Server = Server;
    }

    void bind(HB9HCR_Sensor* Sensor) {
        _Sensor = Sensor;
    }

    void bind(USBCDC* Serial) {
        _Serial = Serial;
    }

    void begin() {
        // initialize servos via UART
        Serial0.begin(1000000, SERIAL_8N1, RX, TX);
        while (!Serial0) delay(100);
        Servo.pSerial = &Serial0;
        Serial.println("actuator: servos initialized on UART bus Serial0");

        // set servos to multi-turn mode
        for (short i = 1; i <= 2; i++) {
            Servo.unLockEprom(i);
            // servo mode 3 (relative)
            Servo.writeByte(i, SMS_STS_MODE, 3);
            // servo angle limits off
            Servo.writeWord(i, 9, 0);
            Servo.writeWord(i, 11, 0);
            Servo.LockEprom(i);
            Servo.EnableTorque(i, 1);
        }

        Serial.println("actuator: servos set to multi-turn");

        if (_Sensor != nullptr) {
            Serial.print("actuator: calibration ");

            float el = 0;

            for (int i = 0; i < 200; i++) {
                _Sensor->read();
                el += _Sensor->degree[1];
                delay(5);
            }

            Serial.printf("done: %.2f\n", el /= 200);
            moveTo(0, -el);
            zero(true, true);
        }

        if (_Server != nullptr) {
            // direct control absolute movement
            _Server->on("/move", HTTP_GET, [this](AsyncWebServerRequest* request) {
                if (!request->hasParam("axis") || !request->hasParam("target")) return;

                float az = 0 == request->getParam("axis")->value().compareTo("az") ? request->getParam("target")->value().toFloat() : 0;
                float el = 0 == request->getParam("axis")->value().compareTo("el") ? request->getParam("target")->value().toFloat() : 0;
                moveTo(az, el);

                JsonDocument data;
                time(&t);

                data["az_degree"] = degree[0];
                data["el_degree"] = degree[1];
                data["az_position"] = position[0];
                data["el_position"] = position[1];
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // direct control relative movement
            _Server->on("/step", HTTP_GET, [this](AsyncWebServerRequest* request) {
                if (!request->hasParam("axis") || !request->hasParam("step")) return;

                int az = 0 == request->getParam("axis")->value().compareTo("az") ? request->getParam("step")->value().toInt() : 0;
                int el = 0 == request->getParam("axis")->value().compareTo("el") ? request->getParam("step")->value().toInt() : 0;
                move(az * STP_DEG, el * STP_DEG);

                JsonDocument data;
                time(&t);

                data["az_degree"] = degree[0];
                data["el_degree"] = degree[1];
                data["az_position"] = position[0];
                data["el_position"] = position[1];
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // direct control zero
            _Server->on("/zero", HTTP_GET, [this](AsyncWebServerRequest* request) {
                if (!request->hasParam("axis")) return;

                bool az = 0 == request->getParam("axis")->value().compareTo("az");
                bool el = 0 == request->getParam("axis")->value().compareTo("el");
                zero(az, el);

                JsonDocument data;
                time(&t);

                data["az_degree"] = degree[0];
                data["el_degree"] = degree[1];
                data["az_position"] = position[0];
                data["el_position"] = position[1];
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            Serial.println("actuator: direct control webhandlers attached");
        }
    }

    void move(int az, int el) {
        // TODO: build in some sort of queueing
        if (0 != az) Servo.WritePosEx(1, az, 0, 50);
        if (0 != el) Servo.WritePosEx(2, el, 0, 50);
        this->position[0] += az;
        this->position[1] += el;
        this->degree[0] = (this->position[0] % STP_TRN) / STP_DEG;
        this->degree[1] = (this->position[1] % STP_TRN) / STP_DEG;
        delay(10);
        while (Servo.ReadMove(1) || Servo.ReadMove(2));
    }

    void moveTo(float az, float el) {
        float shortest = this->shortest(this->degree[0], az);
        float longest = this->longest(shortest);
        this->move((abs(this->position[0] + shortest) < AZ_MAX ? shortest : longest) * STP_DEG, el * STP_DEG - this->position[1]);
    }

    float longest(float delta) {
        if (delta > 0) return delta - 360.0f;
        if (delta < 0) return delta + 360.0f;
        return 0;
    }

    float shortest(float current, float target) {
        float delta = target - current;
        while (delta <= -180.0f) delta += 360.0f;
        while (delta > 180.0f) delta -= 360.0f;
        return delta;
    }

    void zero(bool az, bool el) {
        if (az) this->position[0] = this->degree[0] = 0;
        if (el) this->position[1] = this->degree[1] = 0;
    }

    bool loop() {
        if (_Serial == nullptr || !_Serial->available()) return;


        _c = Serial.read();

        if ('\n' != _c) {
            _command = _command + _c;
            return;
        }

        // set position
        if (strncmp("P", _command.c_str(), 1)) {
            // TODO: set position, parse command
        }

        // get position
        if (strncmp("p", _command.c_str(), 1)) {
            stream->printf("%.2f %.2f", degree[0], degree[1]);
        }

        // move rotator
        if (strncmp("M", _command.c_str(), 1)) {
            // TODO: move
        }

        // stop
        if (strncmp("S", _command.c_str(), 1)) {
            Servo.WriteSpe(1, 0, 0);
            Servo.WriteSpe(2, 0, 0);
        }

        // park
        if (strncmp("K", _command.c_str(), 1)) {
            moveTo(0, 0);
        }

        // set configuration parameter
        if (strncmp("C", _command.c_str(), 1)) {
            // TODO: set configuration parameter
        }

        // get configuration parameter
        if (strncmp("c", _command.c_str(), 1)) {
            // TODO: get configuration parameter
        }

        _command = "";
    }
};

#endif