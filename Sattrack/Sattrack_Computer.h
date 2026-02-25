#ifndef SATTRACK_COMPUTER
#define SATTRACK_COMPUTER

#include "Sattrack_Display.h"
#include "Sattrack_Keypad.h"
#include "Sattrack_Clock.h"

class Sattrack_Computer {
   private:
    Sattrack_Display _display = Sattrack_Display(0x27);
    Sattrack_Keypad _keypad = Sattrack_Keypad();
    Sattrack_Clock _clock = Sattrack_Clock();
    uint32_t _memory[100];
    uint32_t _buffer[3];
    uint8_t _word[3] = {13, 1, 0};
    uint8_t _mode = NONE;
    uint8_t _i;
    char _scr_buffer[9];
    char _scr_word[3];
    char _input = '\0';
    enum {
        VERB,
        NOUN,
        PROG,
        LN_0,
        LN_1,
        LN_2,
        NONE
    };

    void display() {
        _display.loop();
    }

    bool execute(uint8_t n, uint8_t v) {
        memset(_buffer, 0, sizeof(_buffer));
        if (!(n < 100 && v < 100)) return false;
        _word[NOUN] = n;
        _word[VERB] = v;
        return true;
    }

    void input() {
        _keypad.loop();

        switch (_input) {
            case 'n':
            case 'v':
            case '0' ... '9':
            case '#':
            case '*':
                this->handle();

            default:
                _input = '\0';
        }
    }

    void handle() {
        if (('n' == _input || 'v' == _input) && (_mode == NONE || _mode == NOUN || _mode == VERB)) {
            _mode = 'n' == _input ? NOUN : VERB;
            _display.indicate(_mode);
            _display.field(_mode, _buffer[_mode]);
            return;
        }

        if (isdigit(_input) && (_mode == NOUN || _mode == VERB)) {
            snprintf(_scr_word, sizeof(_scr_word), "%02d", _buffer[_mode]);
            _scr_word[0] = _scr_word[1];
            _scr_word[1] = _input;
            _scr_word[2] = '\0';
            _buffer[_mode] = atoi(_scr_word);
            _display.field(_mode, _buffer[_mode]);
            return;
        }

        if ('*' == _input && (_mode == NOUN || _mode == VERB)) {
            _buffer[_mode] = 0;
            _display.field(_mode, _buffer[_mode]);
            return;
        }

        if ('#' == _input && (_mode == NOUN || _mode == VERB)) {
            if (this->execute(_buffer[NOUN], _buffer[VERB])) {
                _display.success();
                _display.clear(false);
                _mode = NONE;
                return;
            }

            _display.error();
            return;
        }
    }

    void process() {}

    void update() {
        _memory[1] = _clock.date();
        _memory[2] = _clock.time();
        _memory[3] = millis();

        if (11 <= _word[VERB] && _word[VERB] <= 13) {
            for (_i = 0; _i <= _word[VERB] - 11; _i++) {
                _display.field(_i + 3, _memory[_word[NOUN] + _i]);
            }
        }
    }

   public:
    Sattrack_Computer() {}

    void init() {
        _display.init();
        _keypad.init(9600, &_input);
        _clock.init();

        _display.label(LN_0, "DATE");
        _display.label(LN_1, "USER");
        _display.label(LN_2, "MS");
        _display.field(NOUN, _word[NOUN]);
        _display.field(VERB, _word[VERB]);
        _display.field(PROG, _word[PROG]);
    }

    void loop() {
        this->input();
        this->handle();
        this->update();
        this->process();
        this->display();
    }
};

#endif