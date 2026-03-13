#ifndef __HB9HCR_ACTUATOR__
#define __HB9HCR_ACTUATOR__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SCServo.h>
#include <string.h>

#include "Sensor.h"

class HB9HCR_Actuator {
   private:
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

    AsyncWebServer* Server = nullptr;
    HB9HCR_Sensor* Sensor = nullptr;
    USBCDC* Input = nullptr;

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

        if (this->Sensor != nullptr) {
            Serial.print("actuator: calibration ");

            float el = 0;

            for (int i = 0; i < 200; i++) {
                this->Sensor->read();
                el += this->Sensor->degree[1];
                delay(5);
            }

            moveTo(0, -(el /= 200));
            zero(true, true);

            Serial.printf("done: %.2f\n", -el);
        }

        if (this->Server != nullptr) {
            Serial.print("actuator: direct control webhandlers ");

            // direct control absolute movement
            this->Server->on("/move", HTTP_GET, [this](AsyncWebServerRequest* request) {
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
            this->Server->on("/step", HTTP_GET, [this](AsyncWebServerRequest* request) {
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
            this->Server->on("/zero", HTTP_GET, [this](AsyncWebServerRequest* request) {
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

            Serial.println("attached");
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

    void loop() {
        if (this->Input == nullptr) return;
        
        while (this->Input->available()) {
            _c = this->Input->read();

            if ('\n' == _c) {
                execute(&_command);
                _command = "";
                return;
            }

            _command = _command + _c;
        }
    }

    void execute(String* command) {
        float az, el;

        // set position
        if (2 == sscanf(command->c_str(), "P %f %f", &az, &el)) {
            moveTo(az, el);
            return;
        }

        // get position
        if (0 == strncmp("p", command->c_str(), 1)) {
            Serial.printf("%.2f %.2f", degree[0], degree[1]);
            return;
        }

        // move rotator
        if (0 == strncmp("M", command->c_str(), 1)) {
            // TODO: move, use sscanf
            return;
        }

        // stop
        if (0 == strncmp("S", command->c_str(), 1)) {
            Servo.WriteSpe(1, 0, 0);
            Servo.WriteSpe(2, 0, 0);
            return;
        }

        // park
        if (0 == strncmp("K", command->c_str(), 1)) {
            moveTo(0, 0);
            return;
        }

        // set configuration parameter
        if (0 == strncmp("C", command->c_str(), 1)) {
            // TODO: set configuration parameter, use sscanf
            return;
        }

        // get configuration parameter
        if (0 == strncmp("c", command->c_str(), 1)) {
            // TODO: get configuration parameter, use sscanf
            return;
        }
    }
};

#endif