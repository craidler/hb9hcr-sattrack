#include "Arduino.h"
#include <Sensor_BMM350.h>

HB9HCR_BMM350 Sensor;
float heading;

void setup() {
    Serial.begin(115200);
    delay(100);
    Sensor.begin();
}

void loop() {
    Sensor.read();
    Serial.printf("hdg: %.2f°\n", Sensor.heading());

    delay(1000);
}