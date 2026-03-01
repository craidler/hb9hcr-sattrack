#include <RTC.h>
#include "Clock.h"

Clock::Clock() {
    RTC.begin();
    this->set(20260301, 120000);
}

uint32_t Clock::date() {
    RTC.getTime(this->_time);
    return this->_time.getYear() * 10000 + Month2int(this->_time.getMonth()) * 100 + this->_time.getDayOfMonth();
}

uint32_t Clock::time() {
    RTC.getTime(this->_time);
    return this->_time.getHour() * 10000 + this->_time.getMinutes() * 100 + this->_time.getSeconds();
}

void Clock::set(uint32_t ux) {
    this->_time.setUnixTime(ux);
    RTC.setTime(this->_time);
}

void Clock::set(uint32_t date, uint32_t time) {
    this->set(this->ux(date, time));
}

uint32_t Clock::ux() {
    RTC.getTime(this->_time);
    return this->_time.getUnixTime();
}

uint32_t Clock::ux(uint32_t date, uint32_t time) {
    struct tm t;
    t.tm_year = date / 10000 - 1900;
    t.tm_mon = (date / 100) % 100 - 1;
    t.tm_mday = date % 100;
    t.tm_hour = time / 10000;
    t.tm_min = (time / 100) % 100;
    t.tm_sec = time % 100;
    t.tm_isdst = -1;
    return mktime(&t);
}