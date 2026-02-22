#ifndef SATTRACK_DISPLAY
#define SATTRACK_DISPLAY

#include <LiquidCrystal_I2C.h>

class Sattrack_Display : public LiquidCrystal_I2C {
  private:
    uint8_t _fq = 200;
    uint8_t _wrd[3] = {0};
    long _act[3] = {false};
    long _reg[3] = {0};
    bool _chg[4] = {true};
    long _last = millis();

  public:
    static const uint8_t WORD_NOUN = 0;
    static const uint8_t WORD_VERB = 1;
    static const uint8_t WORD_PROG = 2;

    Sattrack_Display(uint8_t addr, uint8_t cols, uint8_t rows)
     : LiquidCrystal_I2C(addr, cols, rows) {}

    void act() {
      _act[0] = 0;
      _act[1] = 0;
    }

    void act(uint8_t i) {
      this->act();
      _act[i] = millis();
    }

    void act(uint8_t i, bool v) {
      this->act();
      _act[i] = v ? millis() : 0;
    }

    void clr() {
      char b[21];
      snprintf(b, sizeof(b), "% 20s", " ");

      for (uint8_t i = 0; i < 3; i++) {
        this->setCursor(0, i + 1);
        this->print(b);
      }
    }

    void reg(uint8_t i, long v) {
      _chg[i + 1] = true;
      _reg[i] = v;
    }

    void wrd(uint8_t i, uint8_t v) {
      _chg[0] = true;
      _wrd[i] = v;
    }

    void setup() {
      this->init();
      this->backlight();
    }

    void update() {
      if (millis() - _last < _fq) return;
      _last = millis();

  
      if (_chg[0] | 0 < _act[0] | 0 < _act[1] | 0 < _act[2]) {
        _chg[0] = false;
        
        char w[4] = "NVP";
        char b[21];
        char p[2] = " ";

        for (uint8_t i = 0; i < 2; i++) {
          if (0 < _act[i]) {
            w[i] = (millis() - _act[i]) / 500 % 2 ? w[i] : ' ';
          }
        }

        if (0 < _act[2]) {
          p[0] = (millis() - _act[2]) / 500 % 2 ? ' ' : byte(255);
        }

        snprintf(b, sizeof(b), "%c%02d %c%02d        %c %c%02d", w[0], _wrd[0], w[1], _wrd[1], p[0], w[2], _wrd[2]);
        this->setCursor(0, 0);
        this->print(b);
      }

      for (uint8_t i = 0; i < 3; i++) {
        if (_chg[i + 1]) {
          _chg[i + 1] = false;
          char b[21];
          snprintf(b, sizeof(b), "% 20d", _reg[i]);
          this->setCursor(0, i + 1);
          this->print(b);
        }
      }
    }
};

#endif