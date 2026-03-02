#ifndef SATTRACK_ACTUATOR
#define SATTRACK_ACTUATOR

#include <Arduino.h>
#include <Servo.h>

class Actuator {
   private:
    Servo servo[2];
    uint32_t _last;

   public:
    enum {
        AXIS_AZ,
        AXIS_EZ,
    } Axis;

    Actuator();
    void init();
    void write(uint8_t servo, int pos);
};

#endif