#ifndef __HB9HCR_SERVO__
#define __HB9HCR_SERVO__

#include <Arduino.h>
#include <SMS_STS.h>

class HB9HCR_Servo {
   private:
    unsigned short ID;

   public:
    SMS_STS* Bus = nullptr;

    static constexpr float RESOLUTION = 4096.0 / 360.0;
    unsigned short acceleration = 10;
    unsigned short speed = 0;
    float degree = 0;
    int position = 0;
    int min = -2047;
    int max = +2047;

    HB9HCR_Servo(short id) : ID(id) {}

    // setup the servo to continuous mode and remove angle limitations
    void begin(SMS_STS* b) {
        Bus = b;
        Bus->unLockEprom(ID);
        Bus->writeByte(ID, SMS_STS_MODE, 3);
        Bus->writeWord(ID, 9, 0);
        Bus->writeWord(ID, 11, 0);
        Bus->LockEprom(ID);
        Bus->EnableTorque(ID, 1);
        delay(10);
    }

    // move home
    void home() {
        to(0);
    }

    // move an amount of steps
    void move(int step) {
        if (position + step < min || position + step > max) return;
        while (moving()) delay(5);
        Bus->WritePosEx(ID, step, speed, acceleration);
        position += step;
        degree = (int((position / HB9HCR_Servo::RESOLUTION) * 100) % 36000) / 100.0;
        delay(5);
    }

    bool moving() {
        return 0 != Bus->ReadMove(ID);
    }

    // normalize
    void reset() {
        position = 0;
        degree = 0;
    }

    // move to a certain angle
    void to(float target) {
        move(int(delta(degree, target) * HB9HCR_Servo::RESOLUTION));
    }

    // the short or long way of an angle (delta) in a 360° circle
    float delta(float c, float t) {
        float d = t - c;
        while (d < -180.0f) d += 360.0f;
        while (d > 180.0f) d -= 360.0f;

        // project position at target
        int p = position + d * HB9HCR_Servo::RESOLUTION;

        // within limits
        if (p <= max && p >= min) return d;

        // out of limits, prevent cable tangling
        if (d > 0) return d - 360.0f;
        if (d < 0) return d + 360.0f;
        return 0;
    }
};

#endif