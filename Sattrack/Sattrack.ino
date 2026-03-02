#include <Wire.h>
#include "Computer.h"
#include "Display.h"

Computer computer = Computer();
Display display = Display(0x27);

void setup() {
    Wire.begin(); 
    Wire.setClock(400000);              // fast mode 400kHz (if faster, sensors wont keep up)
    Wire.setWireTimeout(3000, true);    // timeout 3000 micros

    display.init();
    display.clear();
}

void loop() {
    computer.input();
    computer.update();
    computer.process();
    computer.refresh(&display);
}