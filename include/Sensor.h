#ifndef __HB9HCR_SENSOR__
#define __HB9HCR_SENSOR__

#include <Arduino.h>

#include "Actuator.h"
#include "Servo.h"
#include "Sensor_BMI160.h"

class HB9HCR_Sensor {
   public:
    HB9HCR_BMI160 BMI160;

    float azimuth, elevation = 0.0f;

    HB9HCR_Sensor() {}

    void begin() {
        Serial.print("sensor  : bmi160 ");
        BMI160.begin();
        delay(100);
        Serial.println("initialized");
    }

    HB9HCR_Sensor* read() {
        elevation = BMI160.read()->pitch();
        return this;
    }
};

#endif