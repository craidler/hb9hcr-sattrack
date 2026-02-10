import math, time
import gpiozero as GPIO


class Motor:
    PROFILE_LINEAR = 'L'
    PROFILE_PLATEAU = 'P'
    PROFILE_SINUS = 'S'
    PROFILE_TRAPEZE = 'T'

    def __init__(self, direction, step, enable, mode):
        self.pin_direction = direction
        self.pin_enable = enable
        self.pin_step = step

        self.control = {
            direction: GPIO.LED(direction),
            enable: GPIO.LED(enable),
            step: GPIO.LED(step),
            mode[0]: GPIO.LED(mode[0]),
            mode[1]: GPIO.LED(mode[1]),
            mode[2]: GPIO.LED(mode[2]),
        }

    '''
    def trapeze(self, steps, delay=.001, delay_max=.01):
        if 100 > abs(steps):
            self.linear(steps, delay)
            return

        direction = 'forward'

        if 0 > steps:
            direction = 'backward'
            steps = abs(steps)

        steps_ramp = min(50, steps / 2)

        for i in range(steps):
            if i < steps_ramp:
                d = delay_max - (i / steps_ramp) * (delay_max - delay)
            elif i >= (steps - steps_ramp):
                c = i - (steps - steps_ramp)
                d = delay + (c / steps_ramp) * (delay_max - delay)
            else:
                d = delay

            self.driver.TurnStep(direction, 1, d)

        return self
    '''

    def turn(self, profile, steps, delay=.001):
        steps = int(steps)

        if 0 == steps:
            return self

        self.write(self.pin_direction, bool(steps))
        self.write(self.pin_enable, True)

        steps = abs(steps)

        if self.PROFILE_LINEAR == profile:
            for i in range(steps):
                self.write(self.pin_step, True)
                time.sleep(delay)
                self.write(self.pin_step, False)
                time.sleep(delay)

        if self.PROFILE_SINUS == profile:
            stretch = 10
            steep = 10
            for i in range(steps):
                x = (i / steps) * 2 - 1
                f = 1 / (1 + math.exp(-steep * (x + .5))) * (1 / (1 + math.exp(steep * (x - .5))))
                d = delay * stretch - (f * (delay * stretch - delay))
                self.write(self.pin_step, True)
                time.sleep(d)
                self.write(self.pin_step, False)
                time.sleep(d)

        self.write(self.pin_enable, False)
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
