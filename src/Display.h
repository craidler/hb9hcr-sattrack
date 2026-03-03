#ifndef HB9HCR_DISPLAY
#define HB9HCR_DISPLAY

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "Computer.h"

class HB9HCR_Display : public LiquidCrystal_I2C {
   private:
    uint32_t last;
    char line[21];

   public:
    HB9HCR_Display(uint8_t addr);
    void backlight(bool on);
    void render(HB9HCR_Computer* computer);
    bool show(HB9HCR_Computer* computer, uint8_t i);
};

#endif