#ifndef __HB9HCR_BMI160__
#define __HB9HCR_BMI160__

#include "DFRobot_BMI160.h"

class HB9HCR_BMI160 {
   private:
    DFRobot_BMI160 Sensor;
    int16_t raw[6];

   public:
    struct data {
        int x;
        int y;
        int z;
        float x_f;
        float y_f;
        float z_f;
    };

    HB9HCR_BMI160() {}
    data a, g;

    void begin() {
        Sensor.I2cInit(0x69);
        delay(100);
    }

    HB9HCR_BMI160* read() {
        Sensor.getAccelGyroData(raw);
        a.x = raw[3];
        a.y = raw[4];
        a.z = raw[5];
        g.x = raw[0];
        g.y = raw[1];
        g.z = raw[2];

        a.x_f = a.x / 16384.0;
        a.y_f = a.y / 16384.0;
        a.z_f = a.z / 16384.0;

        return this;
    }

    float pitch() {
        // return atan2(a.y_f, sqrt(a.x_f * a.x_f + a.z_f * a.z_f)) * 180.0 / PI; // y points forward
        return atan2(a.z_f, sqrt(a.x_f * a.x_f + a.y_f * a.y_f)) * 180.0 / PI;  // z points forward
    }
};

#endif