#include <DFRobot_DS1307.h>

#include "src/Computer.h"
#include "src/Display.h"
#include "src/Sensor.h"

HB9HCR_Computer Computer;
HB9HCR_Display Display = HB9HCR_Display(0x27);
HB9HCR_Sensor Sensor;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    Sensor.init();
    Display.init();
    Display.clear();
    Display.backlight(true);
}

void loop() {
    Sensor.read(&Computer);
    Sensor.saturate(&Computer);
    Computer.input();
    Computer.update();
    Computer.process();
    Display.render(&Computer);
}