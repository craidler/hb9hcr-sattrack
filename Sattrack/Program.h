#ifndef SATTRACK_P
#define SATTRACK_P

#include "Computer.h"

class Computer;

class Program {
   protected:
    Computer* computer;
    void mem(uint8_t i, uint32_t v);
    uint32_t mem(uint8_t i);
   
   public:
    Program(Computer* c);
    bool complete;
    void init();
    void process();
};

#endif