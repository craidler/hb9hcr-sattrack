#include "Sattrack_Computer.h"

Sattrack_Computer Computer = Sattrack_Computer();

void setup() {
  Computer.init();
}

void loop() {
  Computer.loop();
}
