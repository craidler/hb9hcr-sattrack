#ifndef SATTRACK_CLOCK
#define SATTRACK_CLOCK

#include <RTC.h>

class Sattrack_Clock {
   private:
    RTCTime _current;
    uint32_t _date, _time;

   public:
    Sattrack_Clock() {}

    void init() {
        RTC.begin();
        this->set(20260101, 200000);
    }

    uint32_t date() {
        RTC.getTime(_current);
        _date = _current.getYear() * 10000;
        _date += Month2int(_current.getMonth()) * 100;
        _date += _current.getDayOfMonth();
        return _date;
    }

    uint32_t time() {
        RTC.getTime(_current);
        _time = _current.getHour() * 10000;
        _time += _current.getMinutes() * 100;
        _time += _current.getSeconds();
        return _time;
    }

    void set(uint32_t date, uint32_t time) {
        int y = date / 10000;
        int m = (date / 100) % 100;
        int d = date % 100;
        int hh = time / 10000;
        int mm = (time / 100) % 100;
        int ss = time % 100;

        struct tm t;
        t.tm_year = y - 1900;
        t.tm_mon = m;
        t.tm_mday = d;
        t.tm_hour = hh;
        t.tm_min = mm;
        t.tm_sec = ss;
        t.tm_isdst = -1;

        time_t time_d = mktime(&t);
        RTCTime _current(time_d);
        RTC.setTime(_current);
    }
};

#endif