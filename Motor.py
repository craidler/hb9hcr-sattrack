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

    def ramp(self, steps, delay_min=.001, delay_max=.01):
        if 0 == steps:
            return

        direction = 'forward'

        if 0 > steps:
            direction = 'backward'
            steps = abs(steps)
        
        steps_ramp = min(20, steps / 2)

        for i in range(steps):
            if i < steps_ramp:
                d = delay_max - (i / steps_ramp) * (delay_max - delay_min)
            elif i >= (steps - steps_ramp):
                c = i - (steps - steps_ramp)
                d = delay_min + (c / steps_ramp) * (delay_max - delay_min)
            else:
                d = delay_min

            self.driver.TurnStep(direction, 1, d)

        self.driver.Stop()        


if __name__ == '__main__':
    motor = Motor(13, 19, 12, (16, 17, 20))
    motor.linear(100)
    time.sleep(.2)
    motor.linear(-100)
    time.sleep(1)
    motor.ramp(100)
    time.sleep(.2)
    motor.ramp(-100)
