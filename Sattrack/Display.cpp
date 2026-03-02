#include <LiquidCrystal_I2C.h>

#include "Display.h"
#include "Computer.h"

Display::Display(uint8_t addr, Computer* c) : _computer(c), LiquidCrystal_I2C(addr, 20, 4) {}

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
    LiquidCrystal_I2C::init();
    this->backlight(true);
}

void Display::print() {
    if (this->mem(0) - this->_last < 200) return;

    snprintf(
        this->_line, 21, "%c%02d %c%02d %c        P%02d",
        this->state(Computer::STATE_VERB) && this->flipflop() ? ' ' : 'V',
        this->verb(),
        this->state(Computer::STATE_NOUN) && this->flipflop() ? ' ' : 'N',
        this->noun(),
        this->state(Computer::STATE_ERROR) ? 'E' : ' ',
        this->prog()
    );

    LiquidCrystal_I2C::setCursor(0, 0);
    LiquidCrystal_I2C::print(this->_line);

    for (this->_i = 0; this->_i < 3; this->_i++) {
        snprintf(this->_line, 21, "%20s", "");

        if (this->state(Computer::STATE_IDLE) && this->verb(1, 3) && this->_i <= this->verb() - 1) {
            snprintf(this->_line, 21, "%-8s%012u",
                this->label(this->noun() + this->_i), 
                this->reg(this->_i)
            );
        }

        if (this->state(Computer::STATE_IDLE) && this->verb(11, 13) && this->_i <= this->verb() - 11) {
            snprintf(this->_line, 21, "%-8s%012u",
                this->label(this->noun() + this->_i), 
                this->reg(this->_i)
            );
        }

        if (this->state(Computer::STATE_DATA) && this->verb(21, 23) && this->_i <= this->verb() - 21) {
            snprintf(this->_line, 21, "%-8s%012u",
                this->data(this->_i) && this->flipflop() ? this->label(this->noun() + this->_i) : "", 
                this->reg(this->_i)
            );
        }

        LiquidCrystal_I2C::setCursor(0, this->_i + 1);
        LiquidCrystal_I2C::print(this->_line);
    }

    this->_last = this->mem(0);
}

uint8_t Display::verb() {
    return this->_computer->verb;
}

bool Display::verb(uint8_t l, uint8_t h) {
    return l <= this->_computer->verb && this->_computer->verb <= h;
}

bool Display::data(uint8_t i) {
    return this->_computer->data == i;
}

uint8_t Display::noun() {
    return this->_computer->noun;
}

uint8_t Display::prog() {
    return this->_computer->prog;
}

uint8_t Display::flipflop() {
    return this->_computer->mem[2];
}

uint32_t Display::mem(uint8_t i) {
    return this->_computer->mem[i];
}

uint32_t Display::reg(uint8_t i) {
    return this->_computer->reg[i];
}

bool Display::state(uint8_t s) {
    return this->_computer->state == s;
}

const char* Display::label(uint8_t i) {
    return this->_computer->lbl[i];
}