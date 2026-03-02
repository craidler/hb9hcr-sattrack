#include "Actuator.h"

Actuator::Actuator() {}

void Actuator::init() {
    this->servo[0].attach(3);
    this->servo[1].attach(4);
}

void Actuator::write(uint8_t servo, int pos) {
    uint32_t m = millis();
    if (m - this->_last < 20) return;
    this->_last = m;
    this->servo[servo].write(pos);
}