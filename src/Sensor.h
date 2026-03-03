#ifndef SATTRACK_SENSOR
#define SATTRACK_SENSOR

#include <DFRobot_BMI160.h>
#include <DFRobot_BMM350.h>
#include <DFRobot_INA219.h>
#include <Wire.h>

#include "Computer.h"

#define HB9HCR_SENSOR_DEBUG false
#define HB9HCR_SENSOR_COMPLEMENT 0.96

class HB9HCR_Sensor {
   private:
    DFRobot_BMI160 BMI160 = DFRobot_BMI160();
    DFRobot_BMM350_I2C BMM350 = DFRobot_BMM350_I2C(&Wire, 0x14);
    DFRobot_INA219_IIC INA219 = DFRobot_INA219_IIC(&Wire, 0x45);
    uint32_t last[2];
    float acc_bias;
    float gyr_bias;

   public:
    HB9HCR_Sensor();
    float temperature;
    float elevation;
    float azimuth;
    float acc[3];
    float gyr[3];
    float mag[3];
    float ina[3];
    void saturate(HB9HCR_Computer* computer);
    void read(HB9HCR_Computer* computer);
    void init();
    void dump();
};

#endif