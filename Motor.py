import math, time
import gpiozero as GPIO


def ease_in_out(i, total_i, max_delay, min_delay):
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


def get_stepper_delay(current_step, total_steps, max_speed, min_speed=1.0):
    """
    Calculates the inter-pulse delay for a sin^2 ease-in/out profile.

    Args:
        current_step (int): The current step index.
        total_steps (int): Total steps in the move.
        max_speed (float): Steps per second at cruising speed.
        min_speed (float): The 'floor' speed to avoid division by zero.

    Returns:
        float: Delay in seconds before the next pulse.
    """
    # 10% Accel, 80% Cruise, 10% Decel
    accel_limit = total_steps * 0.10
    decel_start = total_steps * 0.90

    # Calculate instantaneous speed based on quadratic sinus
    if current_step < accel_limit:
        phase = (current_step / accel_limit) * (math.pi / 2)
        speed = max_speed * (math.sin(phase) ** 2)

    elif current_step < decel_start:
        speed = max_speed

    else:
        decel_progress = (current_step - decel_start) / accel_limit
        phase = (math.pi / 2) + (decel_progress * (math.pi / 2))
        speed = max_speed * (math.sin(phase) ** 2)

    # Ensure speed never hits absolute zero to avoid ZeroDivisionError
    actual_speed = max(speed, min_speed)

    return 1.0 / actual_speed

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
                # d = ease_in_out(i, steps, delay_end, delay)
                d = get_stepper_delay(i, steps, 100, 1)
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
