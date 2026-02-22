#include <Wire.h> 
#include <RTC.h>
#include "Sattrack_Computer.h"
#include "Sattrack_Display.h"
#include "Sattrack_Input_Serial.h"

Sattrack_Display Display = Sattrack_Display(0x27, 20, 4);
Sattrack_Computer Computer = Sattrack_Computer(Display);
Sattrack_Input Input = Sattrack_Input();

void setup() {
  Input.setup();
  Computer.setup();
  Display.setup();  
}

void loop() {
    if (Input.available()) {
      Computer.handle(Input.read());
    }

    Computer.update();
    Display.update();
}