#ifndef SATTRACK_SENSOR
#define SATTRACK_SENSOR

#include <MadgwickAHRS.h>
#include <DFRobot_BMI160.h>
#include <DFRobot_BMM350.h>
#include <DFRobot_INA219.h>

class Sensor {
   private:
    DFRobot_BMI160 _bmi160 = DFRobot_BMI160();
    DFRobot_BMM350_I2C _bmm350 = DFRobot_BMM350_I2C(&Wire, 0x14);
    DFRobot_INA219_IIC _ina219 = DFRobot_INA219_IIC(&Wire, 0x40);
    Madgwick _filter;
    uint32_t _current;
    uint32_t _last;
    float _dt = .0;

   public:
    Sensor();
    void init();
    bool read();
    int16_t data[13];
    float pitch = .0;
    float roll = .0;
    float yaw = .0;
};

#endif