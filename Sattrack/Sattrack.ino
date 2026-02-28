#include <LiquidCrystal_I2C.h>
#include <RTC.h>
#include <string.h>

char Label[100][11];

typedef struct {
    RTClock device = RTClock();
    RTCTime time;
} Clock;

typedef struct {
    LiquidCrystal_I2C device;
    uint32_t last = millis();
    uint32_t tot = millis();
    uint8_t i;
    bool backlight = false;
    char scrub[21];
} Display;

typedef struct {
} Gyrometer;

typedef struct {
} Magnetometer;

typedef struct {
} Powermeter;

typedef enum {
    IDLE,
    VERB,
    NOUN,
    DATA,
    ERROR,
} State;

typedef struct {
    Display display = {LiquidCrystal_I2C(0x27, 20, 4)};
    State state = IDLE;
    Clock clock;
    uint32_t buffer;
    uint32_t mem[100];
    uint32_t reg[3];
    uint32_t last;
    uint8_t digit;
    uint8_t limit;
    uint8_t verb;
    uint8_t noun;
    uint8_t prog;
    uint8_t data;
    uint8_t i;
    char input;
} Computer;

Computer computer = {};

void input(Computer* computer) {
    if (!Serial.available()) return;
    handle(computer, Serial.read());
}

void handle(Computer* c, char k) {
    c->display.tot = millis();

    if ('v' == k && IDLE == c->state) {
        c->noun = c->buffer;
        c->buffer = c->digit = 0;
        c->state = VERB;
        return;
    }

    if ('n' == k && VERB == c->state) {
        c->verb = c->buffer;
        c->buffer = c->digit = 0;
        c->state = NOUN;
        return;
    }

    if (VERB == c->state || NOUN == c->state) {
        if (isdigit(k)) {
            c->buffer = (c->buffer * 10) + (k - '0');
            if (++c->digit >= 2) c->buffer %= (int)pow(10, c->digit--);
            return;
        }

        if ('*' == k) {
            c->buffer = c->digit = 0;
            return;
        }

        if ('#' == k) {
            if (VERB == c->state) c->verb = c->buffer;
            if (NOUN == c->state) c->noun = c->buffer;
            c->buffer = c->digit = 0;
            c->state = IDLE;

            // display clear
            clear(&c->display);

            // display value(s)
            if (1 <= c->verb && c->verb <= 3) {
                for (c->i = 0; c->i < c->verb; c->i++) {
                    snprintf(c->display.scrub, sizeof(c->display.scrub), "%-12s%08d", Label[c->noun + c->i], c->mem[c->noun + c->i]);
                    c->display.device.setCursor(0, c->i + 1);
                    c->display.device.print(c->display.scrub);
                }

                return;
            }

            // data input
            if (21 <= c->verb && c->verb <= 23) {
                for (c->i = 0; c->i <= c->verb - 21; c->i++) {
                    snprintf(c->display.scrub, sizeof(c->display.scrub), "%-12s%08d", Label[c->noun + c->i], c->mem[c->noun + c->i]);
                    c->display.device.setCursor(0, c->i + 1);
                    c->display.device.print(c->display.scrub);
                }

                c->state = DATA;
                c->digit = c->data = 0;
                c->limit = c->verb - 21;
                return;
            }

            // terminate
            if (34 == c->verb) {
                c->prog = c->verb = c->noun = 0;
                return;
            }

            // reset
            if (36 == c->verb) {
                NVIC_SystemReset();
                return;
            }

            // change program
            if (37 == c->verb) {
                c->prog = c->noun;
                c->verb = 0;
                c->noun = 0;
                return;
            }

            return;
        }
    }

    if (DATA == c->state) {
        if (isdigit(k)) {
            if (++c->digit <= 10) {
                c->buffer = (c->buffer * 10) + (k - '0');
                c->buffer %= (int)pow(10, c->digit);
                c->reg[c->data] = c->buffer;
            }

            return;
        }

        if ('*' == k) {
            c->reg[c->data] = c->mem[c->noun];
            c->buffer = 0;
            c->digit = 0;
            return;
        }

        if ('#' == k && c->data < c->limit) {
            c->reg[c->data++] = c->buffer;
            c->buffer = c->reg[c->data];
            c->digit = 0;
            return;
        }

        if ('#' == k) {
            // display clear
            clear(&c->display);

            for (c->i = 0; c->i <= c->limit; c->i++) {
                if (4 == c->noun + c->i) date(&c->clock, c->reg[c->i]);
                if (5 == c->noun + c->i) time(&c->clock, c->reg[c->i]);
                c->mem[c->noun + c->i] = c->reg[c->i];
            }

            c->state = IDLE;
            c->verb = c->noun = c->buffer = c->digit = c->limit = 0;
            c->reg[0] = c->reg[1] = c->reg[2] = 0;
            return;
        }
    }
}

void loop() {
    input(&computer);
    process(&computer);
    render(&computer);
}

void process(Computer* c) {
    c->mem[2] = 1000000 / (micros() - c->last);
    c->last = micros();
    c->mem[0] = millis();
    c->mem[1] = c->mem[0] / 500 % 2;
    c->mem[3] = timestamp(&c->clock);
    c->mem[4] = date(&c->clock);
    c->mem[5] = time(&c->clock);

    // program execution in background
    switch (c->prog) {
        case 50:
            program_50(c);
            return;

        default:
            return;
    }
}

void setup() {
    strcpy(Label[0], "MS");
    strcpy(Label[1], "MS ALT");
    strcpy(Label[2], "HZ");
    strcpy(Label[3], "TS");
    strcpy(Label[4], "DATE");
    strcpy(Label[5], "TIME");
    strcpy(Label[50], "AOS TIME");
    strcpy(Label[51], "AOS AZ");
    strcpy(Label[52], "AOS EL");
    strcpy(Label[53], "LOS TIME");
    strcpy(Label[54], "LOS AZ");
    strcpy(Label[55], "LOS EL");
    strcpy(Label[56], "MAX EL");

    Serial.begin(9600);
    init(&computer);
}

void init(Computer* c) {
    c->clock.device.begin();
    set(&c->clock, 20260227, 100000);
    c->display.device.init();
    c->display.tot = millis();
}

// DISPLAY
void clear(Display* d) {
    snprintf(d->scrub, sizeof(d->scrub), "% 20c", ' ');

    for (d->i = 1; d->i <= 3; d->i++) {
        d->device.setCursor(0, d->i);
        d->device.print(d->scrub);
    }
}

void render(Computer* c) {
    if (c->mem[0] - c->display.last < 200) return;
    c->display.last = c->mem[0];

    // backlight on
    if (!c->display.backlight && 0 != c->display.tot) {
        c->display.device.setBacklight(true);
        c->display.backlight = true;
    }

    // indicators
    snprintf(c->display.scrub, sizeof(c->display.scrub), "%c%02d %c%02d %c        P%02d",
             VERB == c->state ? (c->mem[1] ? 'V' : ' ') : 'V',
             VERB == c->state ? c->buffer : c->verb,
             NOUN == c->state ? (c->mem[1] ? 'N' : ' ') : 'N',
             NOUN == c->state ? c->buffer : c->noun,
             ERROR == c->state ? (c->mem[1] ? 'E' : ' ') : ' ',
             c->prog);
    c->display.device.setCursor(0, 0);
    c->display.device.print(c->display.scrub);

    // monitor value(s)
    if (IDLE == c->state && 11 <= c->verb && c->verb <= 13) {
        for (c->i = 0; c->i <= c->verb - 11; c->i++) {
            snprintf(c->display.scrub, sizeof(c->display.scrub), "%-10s%10d", Label[c->noun + c->i], c->mem[c->noun + c->i]);
            c->display.device.setCursor(0, c->i + 1);
            c->display.device.print(c->display.scrub);
        }

        return;
    }

    // register
    if (DATA == c->state && 21 <= c->verb && c->verb <= 23) {
        for (c->i = 0; c->i <= c->verb - 21; c->i++) {
            snprintf(c->display.scrub, sizeof(c->display.scrub), "%-10s%10d",
                     c->data == c->i ? (c->mem[1] ? Label[c->noun + c->i] : "") : Label[c->noun + c->i],
                     c->reg[c->i]);
            c->display.device.setCursor(0, c->i + 1);
            c->display.device.print(c->display.scrub);
        }

        return;
    }

    // backlight off
    if (0 != c->display.tot && c->mem[0] - c->display.tot > 10000) {
        c->display.device.setBacklight(false);
        c->display.backlight = false;
        c->display.tot = 0;
    }
}

// CLOCK
uint32_t date(Clock* c) {
    c->device.getTime(c->time);
    return c->time.getYear() * 10000 + Month2int(c->time.getMonth()) * 100 + c->time.getDayOfMonth();
}

void date(Clock* c, uint32_t v) {
    set(c, v, time(c));
}

uint32_t time(Clock* c) {
    c->device.getTime(c->time);
    return c->time.getHour() * 10000 + c->time.getMinutes() * 100 + c->time.getSeconds();
}

void time(Clock* c, uint32_t v) {
    set(c, date(c), v);
}

uint32_t timestamp(Clock* c) {
    c->device.getTime(c->time);
    return c->time.getUnixTime();
}

uint32_t timestamp(uint32_t date, uint32_t time) {
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

void set(Clock* c, uint32_t date, uint32_t time) {
    c->time.setUnixTime(timestamp(date, time));
    c->device.setTime(c->time);
}

// PROGRAM 50 Satellite Tracking
void program_50(Computer* c) {
    enum {
        P50_IDLE,
        P50_INIT,
        P50_STBY,
        P50_TRCK,
        P50_HOME,
    };

    // start
    if (State::IDLE == c->mem[90]) {
        c->mem[90] = P50_INIT;
        return;
    }

    // initialize
    if (P50_INIT == c->mem[90]) {
        c->mem[90] = P50_STBY;
        c->mem[92] = timestamp(c->mem[4], c->mem[50]);  // aos timestamp
        c->mem[93] = timestamp(c->mem[4], c->mem[53]);  // los timestamp
        // todo validation or c->state = ERROR
        return;
    }

    // wait for aos
    if (P50_STBY == c->mem[90]) {
        // move to az/el

        c->mem[91] = c->mem[92] - c->mem[3];  // countdown
        if (c->mem[92] > c->mem[3]) return;
        c->mem[90] = P50_TRCK;
        return;
    }

    // track until los
    if (P50_TRCK == c->mem[90]) {
        c->mem[91] = c->mem[93] - c->mem[3];  // countdown
        if (c->mem[93] > c->mem[3]) return;
        c->mem[90] = P50_HOME;
        return;
    }

    if (P50_HOME == c->mem[90]) {
        // move to az/el        
        c->mem[90] = c->mem[91] = c->mem[92] = c->mem[93] = 0;
        c->prog = 0;
        return;
    }
}