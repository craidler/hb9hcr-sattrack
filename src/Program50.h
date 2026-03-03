#ifndef HB9HCR_PROGRAM50
#define HB9HCR_PROGRAM50

#include "Computer.h"

class HB9HCR_Program50 : public HB9HCR_Program {
   private:
    HB9HCR_Computer* computer;
    enum uint8_t {
        P50_INIT,
        P50_PREP,
        P50_STBY,
        P50_TRCK,
        P50_HOME,
    } state = P50_INIT;

   public:
    HB9HCR_Program50(HB9HCR_Computer* computer);
    bool terminate();
    bool process();
    bool init();
    bool is(uint8_t state);
    int16_t azimuth();
    uint16_t azimuth(float progress);
    int16_t elevation();
    uint16_t elevation(float progress);
};

#endif