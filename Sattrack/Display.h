#ifndef SATTRACK_DISPLAY
#define SATTRACK_DISPLAY

#include <LiquidCrystal_I2C.h>
#include "Computer.h"

class Computer;

class Display : public LiquidCrystal_I2C {
   private:
    uint32_t _backlight;
    uint32_t _last;
    uint8_t _i;
    char _label[100][9];
    char _line[21];
    void backlight(bool o);

   public:
    Display(uint8_t addr);
    void init();
    void print(Computer* c);
};

#endif