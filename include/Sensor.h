#ifndef __HB9HCR_SENSOR__
#define __HB9HCR_SENSOR__

#include <DFRobot_BMI160.h>
// #include <DFRobot_BMM350.h>
// #include <DFRobot_INA219.h>

class HB9HCR_Sensor {
   private:
    DFRobot_BMI160 BMI160;
    // DFRobot_BMM350_I2C BMM350 = DFRobot_BMM350_I2C(&Wire, 0x14);
    // DFRobot_INA219_IIC INA219 = DFRobot_INA219_IIC(&Wire, 0x45);
    float offset[2] = {0};
    float scale[2] = {1};
    float ax_g, ay_g, az_g;
    int16_t imu[6] = {0};

   public:
    float az, el;

    HB9HCR_Sensor() {}

    void begin() {
        Serial.print("sensor: bmi160 ");
        BMI160.softReset();
        delay(100);

        while (BMI160.I2cInit(0x69)) {
            Serial.println("failed");
            delay(500);
        }

        Serial.println("initialized");
    }

    HB9HCR_Sensor* read() {
        // calculate elevation
        BMI160.getAccelGyroData(imu);
        ax_g = imu[3] / 16384.0;
        ay_g = imu[5] / 16384.0;
        az_g = imu[4] / 16384.0;
        el = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
        return this;
    }
};

#endif