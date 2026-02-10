import math, time
import gpiozero as GPIO


class Motor:
    PROFILE_TRAPEZE = 'trapeze'
    PROFILE_PLATEAU = 'plateau'
    PROFILE_LINEAR = 'linear'
    PROFILE_SINUS = 'sinus'

    def __init__(self, direction, step, enable, mode):
        self.control = {
            direction: GPIO.LED(direction),
            enable: GPIO.LED(enable),
            step: GPIO.LED(step),
            mode[0]: GPIO.LED(mode[0]),
            mode[1]: GPIO.LED(mode[1]),
            mode[2]: GPIO.LED(mode[2]),
        }

    def turn(self, profile, steps, delay=.001):
        steps = int(steps)

        if 0 == steps:
            return self

        self.write('direction', bool(steps))
        self.write('enable', True)

        steps = abs(steps)

        if self.PROFILE_LINEAR == profile:
            for i in range(steps):
                self.write('step', True)
                time.sleep(delay)
                self.write('step', False)
                time.sleep(delay)

        if self.PROFILE_SINUS == profile:
            stretch = 10
            steep = 10
            for i in range(steps):
                x = (i / steps) * 2 - 1
                f = 1 / (1 + math.exp(-steep * (x + .5))) * (1 / (1 + math.exp(steep * (x - .5))))
                d = delay * stretch - (f * (delay * stretch - delay))
                self.write('step', True)
                time.sleep(d)
                self.write('step', False)
                time.sleep(d)

        self.write('enable', False)
        return self

    def write(self, pin, value):
        if value:
            self.control[pin].on()
            return self

        self.control[pin].off()
        return self


if __name__ == '__main__':
    motor = Motor(13, 19, 12, (16, 17, 20))
    motor.turn(motor.PROFILE_LINEAR, 400)
    time.sleep(.2)
    motor.turn(motor.PROFILE_LINEAR, -400)
    time.sleep(1)
    motor.turn(motor.PROFILE_SINUS, 400)
    time.sleep(.2)
    motor.turn(motor.PROFILE_SINUS, -400)
