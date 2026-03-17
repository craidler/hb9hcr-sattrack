#ifndef __HB9HCR_CLOCK__
#define __HB9HCR_CLOCK__

#include <ESP32Time.h>

class HB9HCR_Clock : public ESP32Time {
   public:
    HB9HCR_Clock() : ESP32Time() {}

    void begin() {
        // TODO: this does not work ... why? rtc does not advance
        ESP32Time::setTime(0, 0, 20, 17, 3, 2026);
        Serial.printf("clock   : set to %ld\n", getEpoch());
    }
};

#endif