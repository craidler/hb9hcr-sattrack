#ifndef SATTRACK_DISPLAY
#define SATTRACK_DISPLAY

#include <LiquidCrystal_I2C.h>
#include "Computer.h"

class Computer;

class Display : public LiquidCrystal_I2C {
   private:
    Computer* _computer;
    uint32_t _backlight;
    uint32_t _last;
    uint8_t _i;
    char _line[21];

    const char* label(uint8_t i);
    uint32_t mem(uint8_t i);
    uint32_t reg(uint8_t i);
    uint8_t flipflop();
    uint8_t verb();
    uint8_t noun();
    uint8_t prog();
    bool state(uint8_t i);
    bool data(uint8_t i);
    bool verb(uint8_t l, uint8_t h);

   public:
    Display(uint8_t addr, Computer* c);
    void init();
    void print();
    void backlight(bool o);
};

#endif