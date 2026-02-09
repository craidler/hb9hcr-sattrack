import time

from DRV8825 import DRV8825


class Motor:
    def __init__(self, dir_pin, step_pin, enable_pin, mode_pins, mode='softward', step='fullstep'):
        self.driver = DRV8825(dir_pin, step_pin, enable_pin, mode_pins)
        self.driver.SetMicroStep(mode, step)

    def linear(self, steps, direction='forward', delay=.001):
        self.driver.TurnStep(direction, steps, delay)
        self.driver.Stop()


if __name__ == '__main__':
    motor = Motor(13, 19, 12, (16, 17, 20))
    motor.linear(100)
    time.sleep(1)
