#ifndef SATTRACK_COMPUTER
#define SATTRACK_COMPUTER

#include <RTC.h>
#include "Sattrack_Display.h"

class Sattrack_Computer {
  private:
    Sattrack_Display &_display;
    uint32_t _last = millis();
    uint32_t _mem[100] = {0};
    uint32_t _reg[3] = {0};
    uint8_t  _buf[2] = {0};
    uint8_t  _wrd[3] = {0};
    uint8_t  _act = 0;
    uint8_t  _hz = 120;

  public:
    Sattrack_Computer(Sattrack_Display &d) : _display(d) {}

    void setup() {
      RTC.begin();
      RTCTime startTime(22, Month::FEBRUARY, 2026, 18, 6, 0, DayOfWeek::SUNDAY, SaveLight::SAVING_TIME_ACTIVE);
      RTC.setTime(startTime);
    }

    void execute(uint8_t v) {
      switch (v) {
        case 1:
          this->program_01(true);
          break;

        default:
          return;
      }

      _wrd[2] = v;
      _display.act(2);
      _display.wrd(2, v);
    }

    void handle(char c) {
      if (c == 'n') {
        _display.act(0);
        _act = 1;
        return;
      }

      if (c == 'v') {
        _display.act(1);
        _act = 2;
        return;
      }

      // clear words
      if (c == '*' && 0 != _act) {
        _buf[_act - 1] = 0;
        _display.wrd(_act - 1, 0);
      }

      // input words
      if (c >= '0' && c <= '9' && 0 != _act) {
        char b[3];
        snprintf(b, sizeof(b), "%02d", _buf[_act - 1]);
        b[0] = b[1];
        b[1] = c;
        b[2] = '\0';
        _buf[_act - 1] = atoi(b);
        _display.wrd(_act - 1, _buf[_act - 1]);
      }

      // enter words
      if (c == '#' && 0 != _act) {
        _act = 0;
        _wrd[0] = _buf[0];
        _wrd[1] = _buf[1];
        _display.wrd(0, _wrd[0]);
        _display.wrd(1, _wrd[1]);
        _display.act();
        _buf[0] = 0;
        _buf[1] = 0;

        // display 1
        if (1 == _wrd[1]) {
         _display.reg(0, _mem[_wrd[0]]);
        }

        // display 2
        if (2 == _wrd[1]) {
          _display.reg(0, _mem[_wrd[0]]);
          _display.reg(1, _mem[_wrd[0] + 1]);
        }

        // display 3
        if (3 == _wrd[1]) {
          _display.reg(0, _mem[_wrd[0]]);
          _display.reg(1, _mem[_wrd[0] + 1]);
          _display.reg(2, _mem[_wrd[0] + 2]);
        }

        // recycle program
        if (32 == _wrd[1] && 0 < _wrd[2]) {
          this->execute(_wrd[2]);
        }
        
        // terminate program
        if (34 == _wrd[1] && 0 < _wrd[2]) {
          _wrd[2] = 0;
          _display.act(2, false);
          _display.wrd(2, _wrd[2]);
        }

        // restart (arduino nano r4)
        if (36 == _wrd[1]) {
          NVIC_SystemReset();
        }

        // execute program
        if (37 == _wrd[1]) {
          _display.wrd(0, 0);
          _display.wrd(1, 0);
          _display.wrd(2, _wrd[0]);
          this->execute(_wrd[0]);
          _wrd[0] = 0;
          _wrd[1] = 0;
        }
      }
    }

    void process() {
      switch(_wrd[2]) {
        case 1:
          this->program_01();
          return;

        default:
          return;
      }
    }

    void program_01(bool init) {
      _mem[99] = 0;
    }

    void program_01() {
      _mem[99]++;
    }

    void update() {
      uint8_t dt = millis() - _last;
      if (dt < 1000 / _hz) return;
      _last = millis();

      // runtime
      _mem[1] = dt;
      _mem[2] = _last;

      // realtime
      RTCTime ct;
      RTC.getTime(ct);
      char cdate[9];
      snprintf(cdate, sizeof(cdate), "%04d%02d%02d", ct.getYear(), ct.getMonth(), ct.getDayOfMonth());
      _mem[5] = atoi(cdate);
      char ctime[7];
      snprintf(ctime, sizeof(ctime), "%02d%02d%02d", ct.getHour(), ct.getMinutes(), ct.getSeconds());
      _mem[6] = atoi(ctime);

      // process program if any running
      this->process();

      // monitor 1
      if (11 == _wrd[1]) {
        _display.reg(0, _mem[_wrd[0]]);
      }

      // monitor 2
      if (12 == _wrd[1]) {
        _display.reg(0, _mem[_wrd[0]]);
        _display.reg(1, _mem[_wrd[0] + 1]);
      }
      
      // monitor 3
      if (13 == _wrd[1]) {
        _display.reg(0, _mem[_wrd[0]]);
        _display.reg(1, _mem[_wrd[0] + 1]);
        _display.reg(2, _mem[_wrd[0] + 2]);
      }
    }
};

#endif