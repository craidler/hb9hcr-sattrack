import math, time
import gpiozero as GPIO


def inertial_ease_in_out(i, total_i, max_delay, min_delay):
    ramp_limit = 0.1
    progress = i / (total_i - 1) if total_i > 1 else 0
    if progress < ramp_limit:
        p = progress / ramp_limit
        factor = (math.cos(p * math.pi) + 1) / 2
        return min_delay + (max_delay - min_delay) * factor
    elif progress <= (1 - ramp_limit):
        return min_delay
    else:
        p = (progress - (1 - ramp_limit)) / ramp_limit
        factor = (1 - math.cos(p * math.pi)) / 2
        return min_delay + (max_delay - min_delay) * factor


class Motor:
    MOVE_LINEAR = 'L'
    MOVE_EASE = 'E'

    def __init__(self, direction, step, enable, mode, size=1):
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

        self.sizes = {
            1: (0, 0, 0),
            2: (1, 0, 0),
            4: (0, 1, 0),
            8: (1, 1, 0),
            16: (0, 0, 1),
            32: (1, 0, 1),
        }

        self.size = size

        j = 0
        for i in self.sizes[size]:
            self.write(self.pin_mode[j], i)
            j = j + 1

    def angle(self, degree, delay=.001, profile=MOVE_LINEAR):
        return self.step(degree / 1.8, delay, profile)

    def step(self, steps, delay=.001, profile=MOVE_LINEAR):
        steps = int(steps * self.size)

        if 0 == steps:
            return self

        self.write(self.pin_direction, int(steps > 0))
        self.write(self.pin_enable, True)

        steps = abs(steps)

        if self.MOVE_LINEAR == profile:
            for i in range(steps):
                self.write(self.pin_step, True)
                time.sleep(delay)
                self.write(self.pin_step, False)
                time.sleep(delay)

        if self.MOVE_EASE == profile:
            delay_end = delay * 20

            for i in range(steps):
                d = inertial_ease_in_out(i, steps, delay_end, delay)
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
    motor = Motor(13, 19, 12, (16, 17, 20), 4)
    motor.step(200, .0001)
    time.sleep(1)
    motor.angle(-360, .0001, Motor.MOVE_EASE)
