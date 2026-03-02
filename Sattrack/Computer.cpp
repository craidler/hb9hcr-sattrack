#include "Computer.h"
#include "Display.h"
#include "Program.h"
#include "Program50.h"

Computer::Computer() {
    Serial.begin(9600);

}

void Computer::input() {
    if (!Serial.available()) return;
    this->handle(Serial.read());
}

void Computer::handle(char input) {
    if (Computer::STATE_IDLE == this->state && 'v' == input) {
        this->verb = 0;
        this->state = Computer::STATE_VERB;
        return;
    }
    
    if (Computer::STATE_VERB == this->state && 'n' == input) {
        this->state = Computer::STATE_NOUN;
        return;
    }

    if (Computer::STATE_VERB == this->state && isdigit(input)) {
        this->verb = (this->verb * 10 + (input - '0')) % 100;
        return;
    }

    if (Computer::STATE_NOUN == this->state && '#' == input) {
        this->execute();
        return;
    }

    if (Computer::STATE_NOUN == this->state && isdigit(input)) {
        this->noun = (this->noun * 10 + (input - '0')) % 100;
        return;
    }

    if (Computer::STATE_DATA == this->state && isdigit(input)) {
        this->reg[this->data] = this->reg[this->data] * 10 + (input - '0') % 1000000000;
        return;
    }

    if (Computer::STATE_DATA == this->state && '#' == input) {
        this->mem[this->noun + this->data] = this->reg[this->data];

        if (0 < this->prompt--) {
            this->data++;
            return;
        }

        this->reg[0] = this->reg[1] = this->reg[2] = 0;
        this->state = Computer::STATE_IDLE;
        this->verb = this->noun = 0;
        return;
    }

    if (Computer::STATE_IDLE == this->state) {
        return;
    }

    if ('*' == input) {
        if (Computer::STATE_ERROR == this->state){
            this->state = Computer::STATE_IDLE;
            this->verb = this->noun = 0;
            return;
        }

        if (Computer::STATE_DATA == this->state) this->reg[this->mem[this->noun + this->data]] = 0;
        if (Computer::STATE_VERB == this->state) this->verb = 0;
        if (Computer::STATE_NOUN == this->noun) this->noun = 0;
        return;
    }
}

void Computer::execute() {
    switch (this->verb) {
        case 1:
        case 2:
        case 3:
            for (this->_i = 0; this->_i < this->verb; this->_i++) this->reg[this->_i] = this->mem[this->noun + this->_i];
            this->state = Computer::STATE_IDLE;
            return;

        case 11:
        case 12:
        case 13:
            this->state = Computer::STATE_IDLE;
            return;

        case 21:
        case 22:
        case 23:
            for (this->_i = 0; this->_i < this->verb - 20; this->_i++) this->reg[this->_i] = this->mem[this->noun + this->_i];
            this->prompt = this->verb - 21;
            this->state = Computer::STATE_DATA;
            this->data = 0;
            return;

        // todo: 32

        case 34:
            if (nullptr != this->program) {
                delete this->program;
                this->program = nullptr;
            }

            this->prog = 0;
            return;

        case 36:
            NVIC_SystemReset();
            return;

        case 37:
            switch (this->noun) {
                case 50:
                    this->program = new Program50(this);
                    break;
                
                default:
                    this->state = Computer::STATE_ERROR;
                    this->verb = this->noun = 0;
                    return;
            }

            this->state = Computer::STATE_IDLE;
            this->prog = this->noun;
            this->verb = this->noun = 0;
            return;

        default:
            this->state = Computer::STATE_ERROR;
            this->verb = this->noun = 0;
            return;
    }
}

void Computer::process() {
    if (this->program == nullptr) return;
    this->program->process();

    if (this->program->complete()) {
        delete this->program;
        this->program = nullptr;
        // this->prog = 0;
    }
}

void Computer::update() {
    this->mem[0] = millis();                    // ms
    this->mem[1] = this->mem[0] - this->_last;  // ms delta
    this->mem[2] = this->mem[0] / 500 % 2;      // flip flop
    this->mem[3]++;                             // loops
    this->mem[4] = this->mem[4];                // loops / second
    this->mem[5] = this->state;                 // state
    this->mem[6] = this->clock.ux();            // unix timestamp
    this->mem[7] = this->clock.date();          // date
    this->mem[8] = this->clock.time();          // time

    if (this->mem[1] > 1000) {
        this->mem[4] = this->mem[3];
        this->mem[3] = 0;
        this->_last = this->mem[0];
    }

    if (this->sensor.read()) {
        // raw
        this->mem[20] = this->sensor.data[0] * 1000;
        this->mem[21] = this->sensor.data[1] * 1000;
        this->mem[22] = this->sensor.data[2] * 1000;
        this->mem[23] = this->sensor.data[3] * 1000;
        this->mem[24] = this->sensor.data[4] * 1000;
        this->mem[25] = this->sensor.data[5] * 1000;
        this->mem[26] = this->sensor.data[6] * 1000;
        this->mem[27] = this->sensor.data[7] * 1000;
        this->mem[28] = this->sensor.data[8] * 1000;
        // processed
        this->mem[30] = this->sensor.data[10];
        this->mem[31] = this->sensor.data[11];
        this->mem[32] = this->sensor.data[12];
        this->mem[33] = this->sensor.data[9];
        this->mem[34] = this->sensor.pitch * 1000;
        this->mem[35] = this->sensor.roll * 1000;
        this->mem[36] = this->sensor.yaw * 1000;
    }

    // monitor
    if (Computer::STATE_IDLE == this->state && 11 <= this->verb && this->verb <= 13) {
        for (this->_i = 0; this->_i < this->verb - 10; this->_i++) this->reg[this->_i] = this->mem[this->noun + this->_i];
    }
}

void Computer::refresh(Display* display) {
    display->print(this);
}
