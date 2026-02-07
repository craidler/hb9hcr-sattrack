import sys, time
import threading
from datetime import datetime

# import RPi.GPIO as GPIO
# from RpiMotorLib import RpiMotorLib

AZ_PINS = [20, 21, (14, 15, 18)]
EL_PINS = [22, 23, (17, 27, 4)]


class Tracker:
    STATE_TRACK = 'TRACK'
    STATE_RESET = 'RESET'
    STATE_READY = 'READY'
    STATE_DONE = 'DONE'

    def __init__(self):
        self.az_0 = prompt('aos azimuth [deg, 315]: ', lambda v: max(0, min(360, int(v)))) or 315
        self.az_1 = prompt('los azimuth [deg, 90]:  ', lambda v: max(0, min(360, int(v)))) or 90
        self.az_c = self.azimuth()
        self.az_d = (200 * 20 * 1) / 360  # step per deg
        self.az_s = 2  # step size
        self.az_t = 0
        self.az_m = None  # RpiMotorLib.A4988Nema(AZ_PINS, AZ_PINS, AZ_PINS, "DRV8825")
        self.el_1 = prompt('max elevation [deg, 45]:', lambda v: max(0, min(90, int(v)))) or 45
        self.el_c = self.elevation()
        self.el_d = (200 * 20 * 1) / 360  # step per deg
        self.el_s = 1  # step size
        self.el_t = 0
        self.el_m = None  # RpiMotorLib.A4988Nema(EL_PINS, EL_PINS, EL_PINS, "DRV8825")
        self.t_c = None
        self.t_d = prompt('duration [sec, 10]:     ', lambda v: max(0, min(3600, int(v)))) or 10
        self.t_0 = prompt('start [Ymd HM, T-60]:   ', lambda v: datetime.strptime(v, '%Y%m%d %H%M').timestamp()) or time.time() + 12
        self.t_1 = self.t_0 + self.t_d
        self.interval = .05
        self.state = self.STATE_READY
        self.imu = None  # IMU

    def azimuth(self):
        return .0

    def elevation(self):
        return .0

    def execute(self):
        stop = threading.Event()

        while not stop.is_set():
            try:
                self.t_c = time.time()
                self.handle() or stop.set()
                time.sleep(self.interval)

            except KeyboardInterrupt:
                stop.set()

    def handle(self):
        if self.STATE_READY == self.state:
            sys.stdout.write('\r[%s] T-%d' % (self.state, self.t_0 - self.t_c))
            sys.stdout.flush()

            if self.t_c >= self.t_0:
                sys.stdout.write('\n')
                sys.stdout.flush()
                self.state = self.STATE_TRACK

        if self.STATE_TRACK == self.state:
            if self.t_c >= self.t_1:
                sys.stdout.write('\r[%s] %.2f°%s %.2f°%s 100%%' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90)))
                sys.stdout.flush()
                self.state = self.STATE_RESET
                return True

            t = (self.t_c - self.t_0) / self.t_d
            self.move(t)
            sys.stdout.write('\r[%s] %.2f°%s %.2f°%s %.2f%%' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90), t * 100))
            sys.stdout.flush()

        if self.STATE_RESET == self.state:
            self.move(0)
            sys.stdout.write('\n[%s] %.2f°%s %.2f°%s' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 360)))
            sys.stdout.flush()
            self.state = self.STATE_DONE
            return False

        return True

    def move(self, t):
        # target
        delta = (self.az_1 - self.az_0 + 180) % 360 - 180
        self.az_t = (self.az_0 + (delta * t)) % 360
        self.el_t = 4 * self.el_1 * t * (1 - t)

        # azimuth
        if self.az_m:
            steps = (self.az_t - self.az_c) * self.az_d
            self.az_m.motor_go(steps > 0, 1, abs(steps), .001, False, .01)

        # elevation
        if self.el_m:
            steps = (self.el_t - self.el_c) * self.el_d
            self.el_m.motor_go(steps > 0, 1, abs(steps), .001, False, .01)

        # update
        self.az_c = self.az_t
        self.el_c = self.el_t


def bearing(deg, m):
    deg %= m
    arrows = ['↑', '↗', '→', '↘', '↓', '↙', '←', '↖']
    index = int((deg + 22.5) // 45) % 8

    if 90 == m:
        index = 2 - index

    return arrows[index]


def prompt(label, callback):
    while True:
        try:
            value = input(label + ' ')
            if value == '':
                return None

            return callback(value)

        except ValueError:
            pass


def main():
    tracker = Tracker()

    if 'y' != input('\naccept [y/n]: ').lower():
        sys.stdout.write('aborted\n')
        sys.stdout.flush()
        return

    tracker.execute()


if __name__ == '__main__':
    main()
