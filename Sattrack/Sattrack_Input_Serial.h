#ifndef SATTRACK_INPUT
#define SATTRACK_INPUT

class Sattrack_Input {
  public:
    Sattrack_Input() {}

    void setup() {
      Serial.begin(9600);
      while (!Serial) { ; }
    }

    bool available() {
      return Serial.available();
    }

    char read() {
      return Serial.read();
    }
};

#endif