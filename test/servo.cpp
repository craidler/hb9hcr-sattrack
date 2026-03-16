#include "Servo.h"

#include <Arduino.h>

SMS_STS Bus;
HB9HCR_Servo Azimuth(1);
HB9HCR_Servo Elevation(2);
String cmd;
char c;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(500);

    Serial0.begin(1000000, SERIAL_8N1, RX, TX);
    while (!Serial0) delay(100);
    Bus.pSerial = &Serial0;

    Azimuth.min = -4096;
    Azimuth.max = +4096;
    Azimuth.begin(&Bus);

    Elevation.min = 0;
    Elevation.max = 1024;
    Elevation.begin(&Bus);
}

void loop() {
    while (Serial.available()) {
        c = Serial.read();

        if ('\n' == c) {
            int az_p, el_p;
            if (2 == sscanf(cmd.c_str(), "P %d %d", &az_p, &el_p)) {
                Azimuth.move(az_p);
                Elevation.move(el_p);
            }

            float az_d, el_d;
            if (2 == sscanf(cmd.c_str(), "D %f %f", &az_d, &el_d)) {
                Azimuth.to(az_d);
                Elevation.to(el_d);
            }

            cmd = "";
            return;
        }

        cmd = cmd + c;
    }
}