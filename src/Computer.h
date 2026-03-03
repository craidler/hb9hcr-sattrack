#ifndef HB9HCR_COMPUTER
#define HB9HCR_COMPUTER

#include <Arduino.h>

#include "Program.h"

#define HB9HCR_COMPUTER_DEBUG = true;

class HB9HCR_Computer {
   private:
    HB9HCR_Program* program;
    uint32_t last[1];
    void handle(char input);

   public:
    enum : uint8_t {
        STATE_IDLE,
        STATE_VERB,
        STATE_NOUN,
        STATE_DATA,
        STATE_MONITOR,
        STATE_ERROR,
    } state = STATE_IDLE;

    uint8_t verb;
    uint8_t noun;
    uint8_t prog;
    uint8_t data;
    float buf[3];
    float mem[100];
    char lbl[100][8];

    HB9HCR_Computer();
    void init();
    void input();
    void execute();
    void update();
    void process();
    bool is(uint8_t state);
    const char* label(uint8_t i);
    void label(uint8_t i, const char* v);
};

#endif