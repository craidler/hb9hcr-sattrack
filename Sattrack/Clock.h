#ifndef SATTRACK_CLOCK
#define SATTRACK_CLOCK

#include <RTC.h>

class Clock {
   private:
    RTCTime _time;

   public:
    Clock();
    uint32_t date();
    uint32_t time();
    uint32_t ux();
    uint32_t ux(uint32_t date, uint32_t time);
    void set(uint32_t ux);
    void set(uint32_t date, uint32_t time);
};

#endif