#include "Display.h"

HB9HCR_Display::HB9HCR_Display(uint8_t addr) : LiquidCrystal_I2C(addr, 20, 4) {}

void HB9HCR_Display::backlight(bool on) {
    LiquidCrystal_I2C::setBacklight(on);
}

void HB9HCR_Display::render(HB9HCR_Computer* computer) {
    if (millis() - this->last < 200) return;
    this->last = millis();

    snprintf(this->line, 21, "%c%02d %c%02d %c        P%02d",
             computer->is(HB9HCR_Computer::STATE_VERB) && computer->mem[2] ? ' ' : 'V',
             computer->verb,
             computer->is(HB9HCR_Computer::STATE_NOUN) && computer->mem[2] ? ' ' : 'N',
             computer->noun,
             computer->is(HB9HCR_Computer::STATE_ERROR) && computer->mem[2] ? 'E' : ' ',
             computer->prog);

    LiquidCrystal_I2C::setCursor(0, 0);
    LiquidCrystal_I2C::print(this->line);

    for (uint8_t i = 0; i < 3; i++) {
        if (this->show(computer, i)) {
            snprintf(this->line, 21, "%-8s%012d",
                     computer->is(HB9HCR_Computer::STATE_DATA) && computer->mem[2] && computer->data == i ? "" : computer->label(computer->noun + i),
                     computer->is(HB9HCR_Computer::STATE_DATA) ? (int)computer->buf[i] : (int)computer->mem[computer->noun + i]);
        } else {
            snprintf(this->line, 21, "%20s", " ");
        }

        LiquidCrystal_I2C::setCursor(0, i + 1);
        LiquidCrystal_I2C::print(this->line);
    }
}

bool HB9HCR_Display::show(HB9HCR_Computer* computer, uint8_t i) {
    if (computer->is(HB9HCR_Computer::STATE_DATA) && i < computer->verb - 20) return true;
    if (computer->is(HB9HCR_Computer::STATE_MONITOR) && i < computer->verb - 10) return true;
    return false;
}