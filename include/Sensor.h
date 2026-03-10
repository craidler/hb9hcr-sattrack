#ifndef __HB9HCR_SENSOR__
#define __HB9HCR_SENSOR__

#include <DFRobot_BMI160.h>
#include <DFRobot_BMM350.h>
#include <DFRobot_INA219.h>

#define HB9HCR_MAG_DECLINATION 0.0
#define ACCEL_SENSITIVITY 16384.0

class HB9HCR_Sensor {
   private:
    DFRobot_BMI160 BMI160;
    DFRobot_BMM350_I2C BMM350 = DFRobot_BMM350_I2C(&Wire, 0x14);
    DFRobot_INA219_IIC INA219 = DFRobot_INA219_IIC(&Wire, 0x45);

   public:
    float el_degree;
    float az_degree;

    HB9HCR_Sensor() {}

    void begin() {
        /*
        while (!this->INA219.begin()) {
            Serial.println("INA219 failed");
            delay(1000);
        }
        Serial.println("INA219 initialized");
        */

        while (this->BMI160.I2cInit(0x69)) {
            Serial.println("BMI160 failed");
            delay(1000);
        }

        Serial.println("BMI160 initialized");

        while (this->BMM350.begin()) {
            Serial.println("BMM350 failed");
            delay(1000);
        }

        BMM350.setOperationMode(eBmm350NormalMode);
        BMM350.setMeasurementXYZ();
        Serial.println("BMM350 initialized");
    }

    void read() {
        int16_t imu[6] = {0};

        this->BMI160.getAccelGyroData(imu);
        float ax_g = imu[3];
        float ay_g = imu[5];
        float az_g = imu[4];

        this->el_degree = (atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI);
        this->az_degree = this->BMM350.getCompassDegree();
    }
};

#endif