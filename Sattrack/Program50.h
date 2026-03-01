#ifndef SATTRACK_P50
#define SATTRACK_P50

#include "Program.h"
#include "Computer.h"

class Program50 : public Program {
   private:
    bool _complete = false;
    enum State {
        INIT,
        PREP,
        TRCK,
        HOME,
    };

   public:
    uint8_t state;
    Program50(Computer* c);
    void process();
    bool complete();
};

#endif