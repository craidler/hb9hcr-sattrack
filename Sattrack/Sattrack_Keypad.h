#ifndef SATTRACK_KEYPAD
#define SATTRACK_KEYPAD

class Sattrack_Keypad {
   private:
    char* _input;

   public:
    Sattrack_Keypad() {}

    void init(uint32_t r, char* c) {
        Serial.begin(r);
        _input = c;
    }

    void loop() {
        if (Serial.available()) *_input = Serial.read();
    }
};

#endif