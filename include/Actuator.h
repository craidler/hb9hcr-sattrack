#ifndef __HB9HCR_ACTUATOR__
#define __HB9HCR_ACTUATOR__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SCServo.h>
#include <string.h>

#include "Sensor.h"

class HB9HCR_Actuator {
   private:
    unsigned short i;
    JsonDocument data;
    String response;
    String command;
    SMS_STS Servo;
    time_t t;
    char c;

   public:
    static constexpr float STP_DEG = 4096.0f / 360.0f;  // steps per degree
    static constexpr int STP_TRN = 4096;                // steps per full turn
    static constexpr int AZ_MAX = 3068;                 // cable tangling prevention
    float az, el;
    long az_p, el_p;

    HB9HCR_Actuator() {}

    AsyncWebServer* Server = nullptr;
    HB9HCR_Sensor* Sensor = nullptr;
    USBCDC* Input = nullptr;

    void begin() {
        // initialize servos via UART
        Serial.print("actuator: servos on uart bus ");
        Serial0.begin(1000000, SERIAL_8N1, RX, TX);
        while (!Serial0) delay(100);
        Servo.pSerial = &Serial0;

        // set servos to multi-turn mode
        for (i = 1; i <= 2; i++) {
            Servo.unLockEprom(i);
            // servo mode 3 (relative)
            Servo.writeByte(i, SMS_STS_MODE, 3);
            // servo angle limits off
            Servo.writeWord(i, 9, 0);
            Servo.writeWord(i, 11, 0);
            Servo.LockEprom(i);
            Servo.EnableTorque(i, 1);
        }

        Serial.println("connected");

        // calibrate actuator elevation
        if (Sensor != nullptr) {
            Serial.print("actuator: calibration ");

            float el = 0;

            for (int i = 0; i < 200; i++) {
                el += Sensor->read()->el;
                delay(5);
            }

            moveTo(0, -(el /= 200));
            zero();

            Serial.printf("done: %.2f\n", -el);
        }

        if (Server != nullptr) {
            Serial.print("actuator: direct control webhandlers ");

            // absolute movement
            Server->on("/move", HTTP_GET, [this](AsyncWebServerRequest* request) {
                if (!request->hasParam("axis") || !request->hasParam("target")) return;

                moveTo(0 == request->getParam("axis")->value().compareTo("az") ? request->getParam("target")->value().toFloat() : 0,
                       0 == request->getParam("axis")->value().compareTo("el") ? request->getParam("target")->value().toFloat() : 0);

                data.clear();
                time(&t);

                data["az_degree"] = az;
                data["el_degree"] = el;
                data["az_position"] = az_p;
                data["el_position"] = el_p;
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // relative movement
            Server->on("/step", HTTP_GET, [this](AsyncWebServerRequest* request) {
                if (!request->hasParam("axis") || !request->hasParam("step")) return;

                move(0 == request->getParam("axis")->value().compareTo("az") ? request->getParam("step")->value().toInt() * STP_DEG : 0,
                     0 == request->getParam("axis")->value().compareTo("el") ? request->getParam("step")->value().toInt() * STP_DEG : 0);

                data.clear();
                time(&t);

                data["az_degree"] = az;
                data["el_degree"] = el;
                data["az_position"] = az_p;
                data["el_position"] = el_p;
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            // normalize
            Server->on("/zero", HTTP_GET, [this](AsyncWebServerRequest* request) {
                data.clear();
                time(&t);
                zero();

                data["az_degree"] = az;
                data["el_degree"] = el;
                data["az_position"] = az_p;
                data["el_position"] = el_p;
                data["timestamp"] = t;

                serializeJson(data, response);

                request->send(200, "application/json", response);
            });

            Serial.println("attached");
        }
    }

    // move axis a given amount of steps
    void move(int az_t, int el_t) {
        // TODO: build in some sort of queueing
        if (0 != az_t) Servo.WritePosEx(1, az_t, 0, 50);
        if (0 != el_t) Servo.WritePosEx(2, el_t, 0, 50);
        az_p += az_t;
        el_p += el_t;
        az = (az_p % STP_TRN) / STP_DEG;
        el = (el_p % STP_TRN) / STP_DEG;
        delay(10);
        while (Servo.ReadMove(1) || Servo.ReadMove(2));
    }

    // move axis to a specific position in degrees
    void moveTo(float az_t, float el_t) {
        float s = shortest(az, az_t);
        float l = longest(s);
        move((abs(az_p + s) < AZ_MAX ? s : l) * STP_DEG, el_t * STP_DEG - el_p);
    }

    // the long way of a angle (delta) in a 360° circle
    float longest(float delta) {
        if (delta > 0) return delta - 360.0f;
        if (delta < 0) return delta + 360.0f;
        return 0;
    }

    // the short way of an angle (delta) in a 360° circle
    float shortest(float current, float target) {
        float delta = target - current;
        while (delta <= -180.0f) delta += 360.0f;
        while (delta > 180.0f) delta -= 360.0f;
        return delta;
    }

    // normalize current state of servos as point zero
    void zero() {
        az_p = az = 0;
        el_p = el = 0;
    }

    // assemble commands from usb
    void loop() {
        if (Input == nullptr) return;

        while (Input->available()) {
            c = Input->read();

            if ('\n' == c) {
                execute();
                command = "";
                return;
            }

            command = command + c;
        }
    }

    // execute commands from usb
    void execute() {
        unsigned short direction, speed;
        float az_t, el_t;

        // set position
        if (2 == sscanf(command.c_str(), "P %f %f", &az_t, &el_t)) {
            moveTo(az_t, el_t);
            return;
        }

        // get position
        if (0 == strncmp("p", command.c_str(), 1)) {
            Serial.printf("%.2f %.2f", az, el);
            return;
        }

        // move rotator, this is quite dangerous as there are no stop switches on the device
        // azimuth can snap cables, elevation servo can hit physics
        if (2 == sscanf(command.c_str(), "M %d %d", direction, speed)) {
            switch (direction) {
                case 1:
                    Servo.WriteSpe(1, 0, 0);
                    Servo.WriteSpe(2, 0, 0);
                    return;

                case 2:
                case 3:
                    Servo.WriteSpe(1, 2 == direction ? speed : -speed, 50);
                    return;

                case 4:
                case 5:
                    Servo.WriteSpe(2, 4 == direction ? speed : -speed, 50);
                    return;

                default:
                    return;
            }
        }

        // stop
        if (0 == strncmp("S", command.c_str(), 1)) {
            Servo.WriteSpe(1, 0, 0);
            Servo.WriteSpe(2, 0, 0);
            return;
        }

        // park
        if (0 == strncmp("K", command.c_str(), 1)) {
            moveTo(0.0, 0.0);
            return;
        }

        // reset (zero)
        if (0 == strncmp("R", command.c_str(), 1)) {
            zero();
            return;
        }
    }
};

#endif