
#include "Computer.h"
#include "Display.h"

Computer computer = Computer();
Display display = Display(0x27);

void setup() {
    display.init();
    display.clear();
}

void loop() {
    computer.input();
    computer.update();
    computer.process();
    computer.refresh(&display);
}