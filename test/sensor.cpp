#include <Arduino.h>
#include "Sensor.h"

HB9HCR_Sensor Sensor;

void setup() {
    Serial.begin(115200);
    Sensor.begin();
}

void loop() {
    Sensor.read();
    Serial.printf("az:%.2f° el:%.2f°\n", Sensor.azimuth, Sensor.elevation);
}