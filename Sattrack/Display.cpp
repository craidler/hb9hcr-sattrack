#include <LiquidCrystal_I2C.h>

#include "Display.h"
#include "Computer.h"

Display::Display(uint8_t addr) : LiquidCrystal_I2C(addr, 20, 4) {}

void Display::backlight(bool o) {
    if (o) {
        if (!this->_backlight) LiquidCrystal_I2C::setBacklight(o);
        this->_backlight = millis();
        return;
    }

    if (this->_backlight) LiquidCrystal_I2C::setBacklight(o);
    this->_backlight = 0;
}

void Display::init() {
    strcpy(this->_label[0], "MS");
    strcpy(this->_label[1], "MS D");
    strcpy(this->_label[2], "FF");
    strcpy(this->_label[3], "LP");
    strcpy(this->_label[4], "LPS");
    strcpy(this->_label[5], "STATE");
    strcpy(this->_label[6], "UX");
    strcpy(this->_label[7], "DATE");
    strcpy(this->_label[8], "TIME");
    LiquidCrystal_I2C::init();
    this->backlight(true);
}

void Display::print(Computer* c) {
    if (c->mem[0] - this->_last < 200) return;

    snprintf(
        this->_line, 21, "%c%02d %c%02d %c        P%02d",
        Computer::STATE_VERB == c->state && c->mem[2] ? ' ' : 'V',
        c->verb,
        Computer::STATE_NOUN == c->state && c->mem[2] ? ' ' : 'N',
        c->noun,
        Computer::STATE_ERROR == c->state ? 'E' : ' ',
        c->prog
    );

    LiquidCrystal_I2C::setCursor(0, 0);
    LiquidCrystal_I2C::print(this->_line);

    for (this->_i = 0; this->_i < 3; this->_i++) {
        snprintf(this->_line, 21, "%20s", "");

        if (Computer::STATE_IDLE == c->state && 1 <= c->verb && c->verb <= 3 && this->_i <= c->verb - 1) {
            snprintf(this->_line, 21, "%-8s%012u",
                c->data == this->_i ? (c->mem[2] ? this->_label[c->noun + this->_i] : "") : this->_label[c->noun + this->_i], 
                c->reg[this->_i]
            );
        }

        if (Computer::STATE_IDLE == c->state && 11 <= c->verb && c->verb <= 13 && this->_i <= c->verb - 11) {
            snprintf(this->_line, 21, "%-8s%012u",
                this->_label[c->noun + this->_i], 
                c->reg[this->_i]
            );
        }

        if (Computer::STATE_DATA == c->state && 21 <= c->verb && c->verb <= 23 && this->_i <= c->verb - 21) {
            snprintf(this->_line, 21, "%-8s%012u",
                this->_label[c->noun + this->_i], 
                c->reg[this->_i]
            );
        }

        LiquidCrystal_I2C::setCursor(0, this->_i + 1);
        LiquidCrystal_I2C::print(this->_line);
    }

    this->_last = c->mem[0];
}