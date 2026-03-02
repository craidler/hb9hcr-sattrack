#ifndef SATTRACK_COMPUTER
#define SATTRACK_COMPUTER

#include <Arduino.h>

#include "Clock.h"
#include "Sensor.h"
#include "Display.h"
#include "Program.h"
#include "Program50.h"

class Display;
class Program;

class Computer {
   private:
    uint32_t _last;
    uint8_t _i;

   public:
    enum State {
        STATE_IDLE,
        STATE_VERB,
        STATE_NOUN,
        STATE_DATA,
        STATE_ERROR,
    };

    Clock clock = Clock();
    Sensor sensor = Sensor();
    Program* program = nullptr;
    uint32_t mem[100];
    uint32_t reg[3];
    uint32_t verb;
    uint32_t noun;
    uint32_t prog;
    uint8_t prompt;
    uint8_t state;
    uint8_t data;

    Computer();

    void input();
    void handle(char input);
    void execute();
    void update();
    void process();
    void refresh(Display* display);
};

#endif