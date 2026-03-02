#include "Computer.h"
#include "Program.h"

Program50::Program50(Computer* c) : Program(c) {}

void Program50::init() {
    this->complete = false;
    this->state = Program50::STATE_INIT;
    // copy labels

    // load configuration
    this->_aos_ux = this->computer->clock.ux(this->mem(7), this->mem(50));
    this->_aos_az = this->mem(51);
    this->_aos_el = this->mem(52);
    this->_los_ux = this->computer->clock.ux(this->mem(7), this->mem(53));
    this->_los_az = this->mem(54);
    this->_los_el = this->mem(55);
    this->_max_el = this->mem(55);
    
    // sanity check

    // precalculate
    this->mem(90, this->state = Program50::STATE_PREP);
    this->mem(91, 0);
    this->mem(92, this->_aos_ux);
    this->mem(93, this->_los_ux);
    this->mem(95, this->mem(93) - this->mem(92));
    this->mem(94, this->mem(92) + this->mem(95));
    this->mem(96, 0);
}

void Program50::process() {
    // read current values from computer memory
    this->_cur_el = this->mem(34) / 1000;
    this->_cur_az = this->mem(36) / 1000;
    this->_cur_ux = this->mem(6);
    // write to computer memory
    this->mem(90, this->state);

    // move to AOS
    if (Program50::STATE_PREP == this->state) {
        if (this->_cur_az != this->_aos_az) return;
        if (this->_cur_el != this->_aos_el) return;
        this->state = Program50::STATE_WAIT;
        return;
    }

    // wait for AOS
    if (Program50::STATE_WAIT == this->state) {
        this->mem(91, this->_aos_ux - this->_cur_ux);
        if (this->_cur_ux < this->_aos_ux) return;
        this->state = Program50::STATE_TRCK;
        return;
    }

    // track
    if (Program50::STATE_TRCK == this->state) {
        this->mem(91, this->_los_ux - this->_cur_ux);
        if (this->_cur_ux < this->_los_ux) return;
        this->state = Program50::STATE_HOME;
        return;
    }

    // move back to AOS
    if (Program50::STATE_HOME == this->state) {
        if (this->_cur_az != this->_aos_az) return;
        if (this->_cur_el != this->_aos_el) return;
        this->complete = true;
        return;
    }
}