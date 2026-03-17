#ifndef __HB9HCR_ACTUATOR__
#define __HB9HCR_ACTUATOR__

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SCServo.h>
#include <string.h>

#include "Clock.h"
#include "Sensor.h"
#include "Servo.h"

class HB9HCR_Actuator {
   private:
    JsonDocument data;
    String response;
    String command;
    SMS_STS Bus;
    time_t ts;
    float az, el;
    char c;

   public:
    HB9HCR_Actuator() {}

    AsyncWebServer* Server = nullptr;
    HB9HCR_Sensor* Sensor = nullptr;
    HB9HCR_Servo Azimuth = HB9HCR_Servo(1);
    HB9HCR_Servo Elevation = HB9HCR_Servo(2);
    HB9HCR_Clock* Clock;
    USBCDC* Input = nullptr;

    void begin() {
        // initialize servos via UART
        Serial.print("actuator: servos on uart bus ");
        Serial0.begin(1000000, SERIAL_8N1, RX, TX);
        while (!Serial0) delay(10);
        Bus.pSerial = &Serial0;

        Azimuth.min = -4096;
        Azimuth.max = +4096;
        Azimuth.begin(&Bus);

        Elevation.min = -1024;
        Elevation.max = +1024;
        Elevation.begin(&Bus);
        Serial.println("connected");

        if (Server != nullptr) {
            Serial.print("actuator: direct control webhandlers ");

            // state
            Server->on("/actuator", HTTP_GET, [this](AsyncWebServerRequest* request) {
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // normalize
            Server->on("/actuator", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
                HB9HCR_Servo* Axis = (data["axis"] == "az") ? &Azimuth : &Elevation;
                Axis->reset();
                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            // move
            AsyncCallbackJsonWebHandler* moveHandler = new AsyncCallbackJsonWebHandler("/actuator", [this](AsyncWebServerRequest* request, JsonVariant& json) {
                data = json.as<JsonObject>();

                if (data.isNull()) {
                    data.clear();
                    data["error"] = "invalid json";
                    serializeJson(data, response);
                    request->send(400, "application/json", response);
                    return;
                }

                HB9HCR_Servo* Axis = (data["axis"] == "az") ? &Azimuth : &Elevation;
                (data["mode"] == "a") ? Axis->to(data["value"]) : Axis->move(((float)data["value"] * HB9HCR_Servo::RESOLUTION));

                serializeJson(*getJson(), response);
                request->send(200, "application/json", response);
            });

            Server->addHandler(moveHandler);

            Serial.println("attached");
        }
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

    // execute commands from usb (hamlib compatible)
    void execute() {
        // move relative
        if (2 == sscanf(command.c_str(), "D %f %f", &az, &el)) {
            Azimuth.move(int(az));
            Elevation.move(int(el));
            return;
        }

        // move to absolute position of azimuth and elevation
        if (2 == sscanf(command.c_str(), "P %f %f", &az, &el)) {
            Azimuth.to(az);
            Elevation.to(el);
            return;
        }

        // run calibration
        if (0 == strncmp("C", command.c_str(), 1)) {
            calibrate();
            return;
        }

        // sensor reading
        if (0 == strncmp("S", command.c_str(), 1)) {
            Sensor->read();
            Serial.printf("az: %.2f° el: %.2f°\n", Sensor->azimuth, Sensor->elevation);
            return;
        }

        // get absolute position of azimuth and elevation
        if (0 == strncmp("p", command.c_str(), 1)) {
            Serial.printf("%.2f %.2f\n", Azimuth.degree, Elevation.degree);
            return;
        }

        // move relative (less dangerous than the HAMLIB implementation)
        if (2 == sscanf(command.c_str(), "M %f %f", &az, &el)) {
            Azimuth.move(int(az));
            Elevation.move(int(el));
            return;
        }

        // park
        if (0 == strncmp("K", command.c_str(), 1)) {
            Azimuth.home();
            Elevation.home();
            return;
        }

        // reset
        if (0 == strncmp("R", command.c_str(), 1)) {
            Azimuth.reset();
            Elevation.reset();
            return;
        }
    }

    void calibrate() {
        // calibrate actuator elevation
        if (Sensor != nullptr) {
            // elevation
            Serial.print("actuator: calibration elevation ");
            Elevation.min = -512;
            Elevation.max = +512;

            Sensor->BMI160.read();
            float x = Sensor->BMI160.a.x_f;
            float y = Sensor->BMI160.a.y_f;
            float z = Sensor->BMI160.a.z_f;
            unsigned int i = 0;

            for (i = 0; i < 200; i++) {
                Sensor->BMI160.read();
                x += Sensor->BMI160.a.x_f;
                y += Sensor->BMI160.a.y_f;
                z += Sensor->BMI160.a.z_f;
            }

            Sensor->BMI160.a.x_f = x / i;
            Sensor->BMI160.a.y_f = y / i;
            Sensor->BMI160.a.z_f = z / i;

            el = Sensor->BMI160.pitch();
            Elevation.move(-el * HB9HCR_Servo::RESOLUTION);
            delay(100);
            Elevation.min = 0;
            Elevation.max = 1024;
            Elevation.reset();
            Serial.printf("done: %.2f°\n", -el);
        }
    }

    JsonDocument* getJson() {
        data.clear();

        data["az_deg"] = Azimuth.degree;
        data["el_deg"] = Elevation.degree;
        data["az_pos"] = Azimuth.position;
        data["el_pos"] = Elevation.position;
        data["time"] = Clock->getEpoch();

        return &data;
    }
};

#endif