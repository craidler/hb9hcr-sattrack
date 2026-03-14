#ifndef __HB9HCR_CLOCK__
#define __HB9HCR_CLOCK__

#include <GravityRtc.h>
#include <time.h>
#include <sys/time.h>

class HB9HCR_Clock {
   private:
    GravityRtc RTC;
    String dt;
    time_t now;
    struct tm t;
    char buffer[20];

   public:
    HB9HCR_Clock() {}

    void begin() {
        RTC.setup();
        // RTC.adjustRtc(F(__DATE__), F(__TIME__));

        // sync the esp32 rtc with the gravity one
        time(t);
        now = mktime(&t);
        struct timeval tv = { .tv_sec = now, .tv_usec = 0 };
        settimeofday(&tv, NULL);

        Serial.printf("clock: synched and set to %ld\n", now);
    }

    bool datetime(String &dt) {
        RTC.read();
        snprintf(buffer, 20, "%04d-%02d-%02d %02d:%02d:%02d",
                 RTC.year,
                 RTC.month,
                 RTC.day,
                 RTC.hour,
                 RTC.minute,
                 RTC.second);
        dt = String(buffer);
        return true;
    }

    bool time(struct tm &t) {
        RTC.read();
        t.tm_year = RTC.year - 1900;
        t.tm_mon = RTC.month - 1;
        t.tm_mday = RTC.day;
        t.tm_hour = RTC.hour;
        t.tm_min = RTC.minute;
        t.tm_sec = RTC.second;
        t.tm_isdst = -1;
        return true;
    }

    bool time(String datetime, struct tm &t) {
        int year, month, day, hour, minute, second;
        int parsed = scanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
        if (6 != parsed) return false;
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        t.tm_isdst = -1;
        return true;
    }
};

#endif