#include "Computer.h"

Program::Program(Computer* c) : computer(c) {}

void Program::init() {}

void Program::process() {}

void Program::mem(uint8_t i, uint32_t v) {
    this->computer->mem[i] = v;
}

uint32_t Program::mem(uint8_t i) {
    return this->computer->mem[i];
}