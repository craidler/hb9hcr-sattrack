#include <LiquidCrystal_I2C.h>
#include <RTC.h>

// timers
enum {
    T_RND,  // render todo: rename
    T_TOT,  // backlight todo: rename
    MODE,   // field
    RUN,    // program
};

// fields, words, modes
enum {
    VERB,
    NOUN,
    PROG,
    LN_0,
    LN_1,
    LN_2,
    NONE,
    ERROR,
};

LiquidCrystal_I2C _display = LiquidCrystal_I2C(0x27, 20, 4);
RTCTime _time;

uint32_t _buffer[6];
uint32_t _memory[100];
uint32_t _field[6];
uint32_t _timer[5];
uint8_t _word[3] = {13, 1, 0};
uint8_t _mode = NONE;
uint8_t _i;
char _scrub[21];
char _input;
bool _show[6];

// initialisation
void setup() {
    RTC.begin();
    set(20260226, 100000);
    Serial.begin();
    _display.init();
    backlight(true);
    field(VERB, verb(13));
    field(NOUN, noun(1));
}

// flow
void execute(uint8_t v, uint8_t n) {
    verb(v);
    noun(n);
    mode(NONE);
}

void execute(uint8_t p) {

}

uint8_t mode() {
    return _mode;
}

uint8_t mode(uint8_t v) {
    _timer[MODE] = millis();
    return _mode = v;
}

void loop() {
    handle();
    update();
    render();
}

void restart() {
    NVIC_SystemReset();
}

void update() {
    _memory[1] = date();
    _memory[2] = time();
    _memory[3] = millis();

    if (11 <= verb() && verb() <= 13) {
        for (_i = 0; _i <= verb() - 11; _i++) field(_i + 3, _memory[noun() + _i]);
    }
}

// clock
uint32_t date() {
    RTC.getTime(_time);
    return _time.getYear() * 10000 + Month2int(_time.getMonth()) * 100 + _time.getDayOfMonth();
}

uint32_t date(uint32_t v) {
    set(v, time());
}

uint32_t time() {
    RTC.getTime(_time);
    return _time.getHour() * 10000 + _time.getMinutes() * 100 + _time.getSeconds();
}

uint32_t time(uint32_t v) {
    set(date(), v);
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
    _time.setUnixTime(time_d);
    RTC.setTime(_time);
}

// input
void handle() {
    if (!(Serial.available())) return;
    _input = Serial.read();

    if ('v' == _input && (NONE == _mode || NOUN == _mode)) {
        field(VERB, _buffer[VERB]);
        mode(VERB);
        return;
    }

    if ('n' == _input && (VERB == _mode)) {
        field(NOUN, _buffer[NOUN]);
        mode(NOUN);
        return;
    }

    if (VERB == _mode || NOUN == _mode) {
        if (isdigit(_input)) {
            char _b[3];
            snprintf(_b, 3, "%2d", _buffer[_mode]);
            _b[0] = _b[1];
            _b[1] = _input;
            _b[2] = '\0';
            field(_mode, _buffer[_mode] = atoi(_b));
            return;
        }

        if ('*' == _input) {
            field(_mode, _buffer[_mode] = 0);
            return;
        }

        if ('#' == _input) {
            execute(_buffer[VERB], _buffer[NOUN]);
            clear();
            return;
        }
    }
}

uint8_t noun() {
    return _word[NOUN];
}

uint8_t noun(uint8_t v) {
    return _word[NOUN] = v;
}

uint8_t verb() {
    return _word[VERB];
}

uint8_t verb(uint8_t v) {
    return _word[VERB] = v;
}

// display
void backlight(bool o) {
    if (o && !_timer[T_TOT]) {
        _display.setBacklight(o);
        _timer[T_TOT] = millis();
    }

    if (!o && _timer[T_TOT]) {
        _display.setBacklight(o);
        _timer[T_TOT] = 0;
    }
}

void clear() {
    snprintf(_scrub, sizeof(_scrub), "% 20c", ' ');
    for (_i = LN_0; _i <= LN_2; _i++) {
        _display.setCursor(0, _i - 2);
        _display.print(_scrub);
        _field[_i] = 0;
        _show[_i] = false;
    }
}

uint32_t field(uint8_t i) {
    return _field[i];
}

uint32_t field(uint8_t i, uint32_t v) {
    _timer[T_TOT] = millis();
    _show[i] = true;
    return _field[i] = v;
}

char indicate(uint8_t i, char v) {
    if (_mode != i || !_timer[MODE]) return v;
    return (millis() - _timer[MODE]) / 500 % 2 ? ' ' : v;
}

void render() {
    if (millis() - _timer[T_TOT] > 10000) backlight(false);
    if (millis() - _timer[T_RND] < 200) return;
    _timer[T_RND] = millis();

    snprintf(_scrub, sizeof(_scrub), "%c%02d %c%02d          P%02d", indicate(VERB, 'V'), field(VERB), indicate(NOUN, 'N'), field(NOUN), field(PROG));
    _display.setCursor(0, 0);
    _display.print(_scrub);

    for (_i = LN_0; _i <= LN_2; _i++) {
        if (!_show[_i]) continue;
        snprintf(_scrub, sizeof(_scrub), "%-12s%08d", "", field(_i));
        _display.setCursor(0, _i - 2);
        _display.print(_scrub);
    }
}