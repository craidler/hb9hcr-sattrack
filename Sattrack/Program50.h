#ifndef SATTRACK_P50
#define SATTRACK_P50

#include "Computer.h"
#include "Program.h"

class Program50 : public Program {
   private:
    enum State {
        STATE_INIT,
        STATE_PREP,
        STATE_WAIT,
        STATE_TRCK,
        STATE_HOME,
    };

    uint32_t _aos_ux;
    uint32_t _aos_az;
    uint32_t _aos_el;
    uint32_t _los_ux;
    uint32_t _los_az;
    uint32_t _los_el;
    uint32_t _max_el;
    uint32_t _cur_ux;
    float _cur_az;
    float _cur_el;

   public:

    Program50(Computer* c);
    State state;
    bool complete;
    void init();
    void process();
};

#endif