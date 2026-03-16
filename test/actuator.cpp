#include <Arduino.h>
#include "Actuator.h"
#include "Sensor.h"

HB9HCR_Actuator Actuator;
HB9HCR_Sensor Sensor;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    Actuator.Sensor = &Sensor;
    Actuator.Input = &Serial;

    Sensor.begin();
    Actuator.begin();
}

void loop() {
    Actuator.loop();
}