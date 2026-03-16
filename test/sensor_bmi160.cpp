#include "Arduino.h"
#include <Sensor_BMI160.h>

HB9HCR_BMI160 Sensor;

void setup() {
    Serial.begin(115200);
    Sensor.begin();
}

void loop() {
    Sensor.read();
    Serial.printf("accelerometer x:%d y:%d z:%d s:%.2f°\n", Sensor.a.x, Sensor.a.y, Sensor.a.z, Sensor.pitch());
}