#ifndef SATTRACK_MEMORY
#define SATTRACK_MEMORY

class Sattrack_Memory {
  private:
    uint32_t _memory[100];

  public:
    Sattrack_Memory() {}

    uint32_t get(uint8_t i) {
      if (sizeof(_memory) - 1 < i) return 0;
      return _memory[i];
    }

    void set(uint8_t i, uint32_t v) {
      if (sizeof(_memory) - 1 < i) return;
      _memory[i] = v;
    }
};

#endif