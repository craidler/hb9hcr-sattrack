import math, time
import gpiozero as GPIO


class Motor:
    PROFILE_LINEAR = 'L'
    PROFILE_PLATEAU = 'P'
    PROFILE_SINUS = 'S'
    PROFILE_TRAPEZE = 'T'

    STEP_01 = (0, 0, 0)
    STEP_02 = (1, 0, 0)
    STEP_04 = (0, 1, 0)
    STEP_08 = (1, 1, 0)
    STEP_16 = (0, 0, 1)
    STEP_32 = (1, 0, 1)

    def __init__(self, direction, step, enable, mode, microstep=STEP_01):
        self.pin_direction = direction
        self.pin_enable = enable
        self.pin_step = step
        self.pin_mode = mode

        self.control = {
            direction: GPIO.LED(direction),
            enable: GPIO.LED(enable),
            step: GPIO.LED(step),
            mode[0]: GPIO.LED(mode[0]),
            mode[1]: GPIO.LED(mode[1]),
            mode[2]: GPIO.LED(mode[2]),
        }

        j = 0
        for i in microstep:
            self.write(self.pin_mode[j], i)
            j = j + 1

    def angle(self, degree, delay=.001, profile=PROFILE_LINEAR):
        return self.step(degree / 1.8, delay, profile)

    def step(self, steps, delay=.001, profile=PROFILE_LINEAR):
        steps = int(steps)

        if 0 == steps:
            return self

        self.write(self.pin_direction, int(steps > 0))
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

        if self.PROFILE_TRAPEZE == profile:
            stretch = 10
            ramp = 50

            for i in range(steps):
                if ramp > i:
                    d = delay + (delay * stretch * (i / ramp))
                elif steps - ramp < i:
                    d = delay + (delay * stretch * ((steps - i) / ramp))
                else:
                    d = delay
                self.write(self.pin_step, True)
                time.sleep(d)
                self.write(self.pin_step, False)
                time.sleep(d)

        if self.PROFILE_PLATEAU == profile:
            stretch = 10
            cruise = .3

            for i in range(steps):
                phase = (i / steps) * 2 * math.pi
                raw_sine = math.sin(phase)
                limit = 1.0 - cruise
                clipped_sine = max(min(-raw_sine, limit), -limit)
                normalized = (clipped_sine + limit) / (2 * limit)
                d = delay + (normalized * (delay * stretch - delay))
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
    motor.step(200)
    time.sleep(1)
    motor.angle(-90)
    time.sleep(1)
    motor.angle(-270)
