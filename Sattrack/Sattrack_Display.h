#ifndef SATTRACK_DISPLAY
#define SATTRACK_DISPLAY

#include <LiquidCrystal_I2C.h>

class Sattrack_Display : public LiquidCrystal_I2C {
   private:
    const static uint16_t TOT = 10000;
    const static uint8_t HZ = 5;
    uint32_t _field[6];
    uint32_t _time[4];
    uint32_t _indicator = 0;
    uint8_t _indicate = 0;
    uint8_t _changed = 1;
    uint8_t _i;
    char _label[3][11];
    char _title[4] = "VNE";
    char _scr[21];
    enum {
        LAST,
        LIGHT,
        EXEC,
        ERROR
    };

    void refresh() {
        if (!_changed && !_indicate) return;

        _i = (millis() - _indicator) / 500 % 2;
        snprintf(_scr, 21, "%c%02d %c%02d          P%02d", _indicate & (1 << VERB) && _i ? ' ' : _title[VERB], _field[VERB], _indicate & (1 << NOUN) && _i ? ' ' : _title[NOUN], _field[NOUN], _field[PROG]);
        this->print(0, 0, _scr);

        for (_i = LN_0; _i <= LN_2; _i++) {
            if (!(_changed & (1 << _i))) continue;
            snprintf(_scr, 21, "%-10s% 10u", _label[_i -3], _field[_i]);
            this->print(0, _i - 2, _scr);
        }

        _changed = 0;
    }

   public:
    Sattrack_Display(uint8_t addr) : LiquidCrystal_I2C(addr, 20, 4) {}
    enum {
        VERB,
        NOUN,
        PROG,
        LN_0,
        LN_1,
        LN_2
    };

    void backlight(bool o) {
        if (o) {
            if (!_time[LIGHT]) LiquidCrystal_I2C::setBacklight(o);
            _time[LIGHT] = millis();
            return;
        }

        if (_time[LIGHT]) LiquidCrystal_I2C::setBacklight(o);
        _time[LIGHT] = 0;
    }

    void clear(bool d) {
        _changed = 0;
        snprintf(_scr, 21, "% 20c", ' ');
        for (_i = LN_0; _i <= LN_2; _i++) {
            _field[_i] = d ? 0 : _field[_i];
            this->print(0, _i - 2, _scr);
            if (d) snprintf(_label[_i], 11, "");
        }
    }

    void error() {
        this->backlight(true);
        _time[ERROR] = millis();
    }

    void field(uint8_t i, uint32_t v) {
        if (i == PROG) _time[EXEC] = millis();
        this->backlight(true);
        _changed |= (1 << i);
        _field[i] = v;
    }

    void indicate(uint8_t i) {
        this->backlight(true);
        _indicator = millis();
        _indicate = (1 << i);
    }

    void init() {
        LiquidCrystal_I2C::init();
        this->backlight(true);
    }

    void label(uint8_t i, const char* v) {
        snprintf(_label[i - 3], 11, v);
    }

    void loop() {
        if (millis() - _time[LIGHT] >= TOT) this->backlight(false);
        if (millis() - _time[LAST] < 1000 / HZ) return;
        _time[LAST] = millis();
        this->refresh();
    }

    void print(uint8_t x, uint8_t y, char* v) {
        LiquidCrystal_I2C::setCursor(x, y);
        LiquidCrystal_I2C::print(v);
    }

    void success() {
        this->backlight(true);
        _indicate = 0;
    }
};

#endif