#ifndef SATTRACK_COMPUTER
#define SATTRACK_COMPUTER

#include <Arduino.h>

#include "Clock.h"
#include "Sensor.h"
#include "Display.h"
#include "Actuator.h"
#include "Program.h"
#include "Program50.h"

class Actuator;
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

    Actuator actuator = Actuator();
    Program* program = nullptr;
    Sensor sensor = Sensor();
    Clock clock = Clock();
    uint32_t mem[100];
    uint32_t reg[3];
    uint32_t verb;
    uint32_t noun;
    uint32_t prog;
    uint8_t prompt;
    uint8_t state;
    uint8_t data;
    char lbl[100][9];

    Computer();

    const char* label(uint8_t i);
    void label(uint8_t i, const char* v);

    void init();
    void input();
    void handle(char input);
    void execute();
    void update();
    void process();
    void refresh(Display* display);
};

#endif