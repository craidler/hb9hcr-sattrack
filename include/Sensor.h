#ifndef __HB9HCR_SENSOR__
#define __HB9HCR_SENSOR__

#include <Arduino.h>

#include "Actuator.h"
#include "Servo.h"
#include "Sensor_BMI160.h"
#include "Sensor_BMM350.h"

class HB9HCR_Sensor {
   public:
    HB9HCR_BMI160 BMI160;
    HB9HCR_BMM350 BMM350;

    float azimuth, elevation;

    HB9HCR_Sensor() {}

    void begin() {
        delay(100);
        Serial.print("sensor  : bmi160 ");
        BMI160.begin();
        Serial.println("initialized");

        delay(100);
        Serial.print("sensor  : bmm350 ");
        BMM350.begin();
        Serial.println("initialized");
    }

    HB9HCR_Sensor* read() {
        elevation = BMI160.read()->pitch();
        azimuth = BMM350.read()->heading();
        return this;
    }
};

#endif