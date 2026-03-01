#ifndef SATTRACK_P
#define SATTRACK_P

#include "Computer.h"

class Computer;

class Program {
   protected:
    Computer* c;

   
   public:
    Program(Computer* c);
    void process();
    bool complete();
};

#endif