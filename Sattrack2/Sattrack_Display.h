#ifndef SATTRACK_DISPLAY
#define SATTRACK_DISPLAY

#include <LiquidCrystal_I2C.h>

class Sattrack_Display : public LiquidCrystal_I2C {
  private:
    const static uint8_t _dt = 200;
    const static uint32_t _tot = 10000;
    uint32_t _last = 0;
    uint32_t _on = 0;
    uint32_t _indicator[6] = {0};
    uint32_t _field[6] = {0};
    uint8_t _expect = 0;
    uint8_t _change = 0;
    char _label[4];
    char _line[21];
    
    void print(uint8_t x, uint8_t y, char* v) {
      LiquidCrystal_I2C::setCursor(x, y);
      LiquidCrystal_I2C::print(v);
    }

    void refresh() {
      if (0 == _change && 0 == _expect) return;
      
      memcpy(_label, "NVP", 4);
      
      for (uint8_t i = 0; i < 3; i++) {
        if (!(_expect & (1 << i))) continue;
        _label[i] = (millis() - _indicator[i]) / 500 % 2 ? _label[i] : ' ';
      }
      
      snprintf(_line, sizeof(_line), "%c%02u %c%02u          %c%02u", _label[0], _field[0], _label[1], _field[1], _label[2], _field[2]);
      this->print(0, 0, _line);

      for (uint8_t i = 3; i < 6; i++) {
        if (!(_change & (1 << i))) continue;
        snprintf(_line, sizeof(_line), "%-12c%08u", ' ', _field[i]);
        this->print(0, i - 2, _line);
      }

      _change = 0;
    }

  public:
    enum { NOUN, VERB, PROG, LINE_0, LINE_1, LINE_2 };
    
    Sattrack_Display(uint8_t addr) : LiquidCrystal_I2C(addr, 20, 4) {}

    void clear() {
      snprintf(_line, sizeof(_line), "% 20c", ' ');
      for (uint8_t y = 1; y < 4; y++) this->print(0, y, _line);
      _expect = 0;
    }

    void expect(uint8_t i) {
      _indicator[i] = millis();
      _expect = (1 << i);
      _on = 0;
    }

    void field(uint8_t i, uint32_t v) {
      if (_field[i] == v) return;
      _change |= (1 << i); 
      _field[i] = v;
    }

    void loop() {
      if (millis() - _last < _dt) return;
      _last = millis();
      this->refresh();
      if (0 < _on && _last > _on + _tot) this->off();
    }
    
    void on() {
      if (0 == _on) this->backlight();
      _on = millis();
    }

    void off() {
      this->noBacklight();
      _on = 0;
    }
};

#endif