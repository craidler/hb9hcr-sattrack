#include "Computer.h"
#include "Program.h"

Program50::Program50(Computer* c) : Program(c) {}

void Program50::process() {
}

bool Program50::complete() {
    return this->_complete;
}