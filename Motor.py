import math
import time

from DRV8825 import DRV8825


class Motor:
    def __init__(self, dir_pin, step_pin, enable_pin, mode_pins, mode='softward', step='fullstep'):
        self.driver = DRV8825(dir_pin, step_pin, enable_pin, mode_pins)
        self.driver.SetMicroStep(mode, step)

    def linear(self, steps, delay=.001):
        if 0 == steps:
            return 

        if 0 < steps:
            self.driver.TurnStep('forward', abs(steps), delay)

        if 0 > steps:
            self.driver.TurnStep('backward', abs(steps), delay)
        
        self.driver.Stop()

    def trapeziodal(self, steps, delay=.001, delay_max=.01):
        if 0 == steps:
            return

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

        self.driver.Stop()  

    def sinusoidal(self, steps, delay=.001, delay_max=0.01):
        if 0 == steps:
            return
        
        direction = 'forward'
        steepness = 10

        if (0 > steps):
            direction = 'backward'
            steps = abs(steps)
        
        for i in range(steps):
            x = (i / steps) * 2 - 1
            f = 1 / (1 + math.exp(-steepness * (x + .5))) * (1 / (1 + math.exp(steepness * (x - .5))))
            d = delay_max - (f * (delay_max - delay))
            self.driver.TurnStep(direction, 1, d)
        
        self.driver.Stop()


if __name__ == '__main__':
    motor = Motor(13, 19, 12, (16, 17, 20), 'softward', '1/32step')
    motor.linear(400)
    time.sleep(.2)
    motor.linear(-400)
    time.sleep(1)
    motor.trapeziodal(400)
    time.sleep(.2)
    motor.trapeziodal(-400)
    time.sleep(1)
    motor.sinusoidal(400)
    time.sleep(.2)
    motor.sinusoidal(-400)
