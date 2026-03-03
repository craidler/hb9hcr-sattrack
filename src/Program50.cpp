#include "Program50.h"

HB9HCR_Program50::HB9HCR_Program50(HB9HCR_Computer* computer) : computer(computer), HB9HCR_Program() {}

bool HB9HCR_Program50::init() {
    this->computer->label(50, "AOS TME");
    this->computer->label(51, "AOS AZ");
    this->computer->label(52, "AOS EL");
    this->computer->label(53, "LOS TME");
    this->computer->label(54, "LOS AZ");
    this->computer->label(55, "LOS EL");
    this->computer->label(56, "MAX EL");
    return true;
}

bool HB9HCR_Program50::process() {
    if (this->is(HB9HCR_Program50::P50_INIT)) {
        this->state = HB9HCR_Program50::P50_PREP;
        return true;
    }

    if (this->is(HB9HCR_Program50::P50_PREP)) {
        this->state = HB9HCR_Program50::P50_STBY;
        return true;
    }

    if (this->is(HB9HCR_Program50::P50_STBY)) {
        this->state = HB9HCR_Program50::P50_TRCK;
        return true;
    }

    if (this->is(HB9HCR_Program50::P50_TRCK)) {
        this->state = HB9HCR_Program50::P50_HOME;
        return true;
    }

    if (this->is(HB9HCR_Program50::P50_HOME)) {
        return false;
    }

    return true;
}

bool HB9HCR_Program50::terminate() {
    return true;
}

int16_t HB9HCR_Program50::azimuth() {
    return this->computer->mem[50];
}

uint16_t HB9HCR_Program50::azimuth(float progress) {
    return 0;
}

int16_t HB9HCR_Program50::elevation() {
    return this->computer->mem[50];
}

uint16_t HB9HCR_Program50::elevation(float progress) {
    return 0;
}

bool HB9HCR_Program50::is(uint8_t state) {
    return state == this->state;
}
