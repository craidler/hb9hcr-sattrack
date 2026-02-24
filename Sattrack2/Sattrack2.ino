#include "Sattrack_Display.h"

Sattrack_Display Display = Sattrack_Display(0x27);

void setup() {
  Display.init();
  Display.on();
  Display.field(Sattrack_Display::LINE_2, 12345);
  Display.expect(Sattrack_Display::NOUN);
}

void loop() {
  Display.field(Sattrack_Display::LINE_1, millis());
  Display.loop();
}