#include "Computer.h"

#include "Program50.h"

HB9HCR_Computer::HB9HCR_Computer() {
    strcpy(this->lbl[0], "MS");
    strcpy(this->lbl[1], "DT");
    strcpy(this->lbl[2], "FF");
    strcpy(this->lbl[3], "LP");
    strcpy(this->lbl[4], "HZ");
    strcpy(this->lbl[5], "STATE");
    strcpy(this->lbl[6], "UXTS");
    strcpy(this->lbl[7], "DATE");
    strcpy(this->lbl[8], "TIME");

    strcpy(this->lbl[10], "BMI160");
    strcpy(this->lbl[11], "BMM350");
    strcpy(this->lbl[12], "INA219");

    strcpy(this->lbl[20], "ACC X");
    strcpy(this->lbl[21], "ACC Y");
    strcpy(this->lbl[22], "ACC Z");
    strcpy(this->lbl[23], "GYR X");
    strcpy(this->lbl[24], "GYR Y");
    strcpy(this->lbl[25], "GYR Z");
    strcpy(this->lbl[26], "MAG X");
    strcpy(this->lbl[27], "MAG Y");
    strcpy(this->lbl[28], "MAG Z");
}

void HB9HCR_Computer::execute() {
    switch (this->verb) {
        case 11:
        case 12:
        case 13:
            this->state = HB9HCR_Computer::STATE_MONITOR;
            return;

        case 21:
        case 22:
        case 23:
            this->state = HB9HCR_Computer::STATE_DATA;
            this->data = 0;
            this->buf[0] = this->mem[this->noun];
            this->buf[1] = this->mem[this->noun + 1];
            this->buf[2] = this->mem[this->noun + 2];
            return;

        case 36:
            NVIC_SystemReset();
            return;

        case 37:
            switch (this->noun) {
                case 50:
                    this->program = new HB9HCR_Program50(this);
                    this->program->init();
                    this->prog = this->noun;
                    this->verb = this->noun = 0;
                    break;

                default:
                    this->state = HB9HCR_Computer::STATE_ERROR;
                    return;
            }

            break;

        default:
            this->state = HB9HCR_Computer::STATE_ERROR;
            return;
    }

    this->state = HB9HCR_Computer::STATE_IDLE;
}

void HB9HCR_Computer::handle(char input) {
    if (this->is(HB9HCR_Computer::STATE_IDLE) && 'v' == input) {
        this->state = HB9HCR_Computer::STATE_VERB;
        this->verb = 0;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_MONITOR) && 'v' == input) {
        this->state = HB9HCR_Computer::STATE_VERB;
        this->verb = 0;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_VERB) && 'n' == input) {
        this->state = HB9HCR_Computer::STATE_NOUN;
        this->noun = 0;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_VERB) && '*' == input) {
        this->verb = 0;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_VERB) && isdigit(input)) {
        this->verb = (this->verb * 10 + (input - '0')) % 100;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_NOUN) && isdigit(input)) {
        this->noun = (this->noun * 10 + (input - '0')) % 100;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_NOUN) && '*' == input) {
        this->noun = 0;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_NOUN) && '#' == input) {
        this->execute();
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_DATA) && isdigit(input)) {
        this->buf[this->data] = (int)(this->buf[this->data] * 10 + (input - '0')) % 1000000000;
        return;
    }

    if (this->is(HB9HCR_Computer::STATE_DATA) && '#' == input) {
        this->mem[this->noun + this->data] = this->buf[this->data];
        this->data++;

        if (this->data >= this->verb - 20) {
            this->state = HB9HCR_Computer::STATE_IDLE;
            this->verb = this->noun = this->data = 0;
        }

        return;
    }

    if (this->is(HB9HCR_Computer::STATE_ERROR) && '*' == input) {
        this->state = HB9HCR_Computer::STATE_IDLE;
        return;
    }
}

void HB9HCR_Computer::init() {
}

void HB9HCR_Computer::input() {
    if (!Serial.available()) return;
    this->handle(Serial.read());
}

bool HB9HCR_Computer::is(uint8_t state) {
    return state == this->state;
}

const char* HB9HCR_Computer::label(uint8_t i) {
    return this->lbl[i];
}

void HB9HCR_Computer::label(uint8_t i, const char* v) {
    strcpy(this->lbl[i], v);
}

void HB9HCR_Computer::update() {
    this->mem[0] = millis();
    this->mem[1] = micros() - this->last[0];
    this->mem[2] = (int)this->mem[0] / 500 % 2;
    this->mem[3]++;
    this->mem[4] = this->mem[4];
    this->mem[5] = this->state;

    if (this->mem[1] > 1000000) {
        this->mem[4] = this->mem[3];
        this->mem[3] = 0;
        this->last[0] = micros();
    }
}

void HB9HCR_Computer::process() {
    if (!this->prog) return;
}