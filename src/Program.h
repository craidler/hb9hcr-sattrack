#ifndef HB9HCR_PROGRAM
#define HB9HCR_PROGRAM

#include "Computer.h"

class HB9HCR_Program {
   public:
    HB9HCR_Program() {}
    virtual ~HB9HCR_Program() = default;
    virtual bool terminate() {}
    virtual bool process() {}
    virtual bool init() {}
};

#endif