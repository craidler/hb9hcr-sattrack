import sys, time
import threading
from datetime import datetime
import gpiozero as GPIO
from DRV8825 import DRV8825

AZ_PINS = (16, 17, 20)
EL_PINS = (21, 22, 27)


class Tracker:
    STATE_ARMED = 'ARMED'
    STATE_RESET = 'RESET'
    STATE_SETUP = 'SETUP'
    STATE_TRACK = 'TRACK'
    STATE_DONE = 'DONE'

    def __init__(self):
        self.az_0 = prompt('aos azimuth    [315°]:', lambda v: max(0, min(360, int(v)))) or 315
        self.az_1 = prompt('los azimuth    [090°]:', lambda v: max(0, min(360, int(v)))) or 90
        self.az_c = self.azimuth()
        self.az_d = (200 * 20 * 1) / 360  # step per deg
        self.az_s = 1  # step size
        self.az_t = 0
        self.az_m = DRV8825(13, 19, 12, AZ_PINS)
        self.el_1 = prompt('max elevation  [045°]:', lambda v: max(0, min(90, int(v)))) or 45
        self.el_c = self.elevation()
        self.el_d = (200 * 20 * 1) / 360  # step per deg
        self.el_s = 1  # step size
        self.el_t = 0
        self.el_m = DRV8825(24, 18, 4, EL_PINS)
        self.t_c = None
        self.t_d = prompt('duration        [10s]:', lambda v: max(0, min(3600, int(v)))) or 10
        self.t_0 = prompt('start [Ymd HM, T-20s]:', lambda v: datetime.strptime(v, '%Y%m%d %H%M').timestamp()) or time.time() + 20
        self.t_1 = self.t_0 + self.t_d
        self.interval = .05
        self.state = self.STATE_SETUP
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
        sys.stdout.flush()

        if self.STATE_SETUP == self.state:
            sys.stdout.write('\n')
            for i in reversed(range(10)):
                sys.stdout.write('\r[%s] %03d°%s %03d°%s T-%03d CLEAR DEVICE' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90), i))
                time.sleep(1)

            self.move(0)
            self.state = self.STATE_ARMED
            sys.stdout.write('\n')
            return True

        if self.STATE_ARMED == self.state:
            sys.stdout.write('\r[%s] %03d°%s %03d°%s T-%03d AOS %s' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90), self.t_0 - self.t_c, datetime.fromtimestamp(self.t_0).strftime('%Y-%m-%d %H:%M')))

            if self.t_c >= self.t_0:
                sys.stdout.write('\n')
                self.state = self.STATE_TRACK
                return True

        if self.STATE_TRACK == self.state:
            if self.t_c >= self.t_1:
                sys.stdout.write(
                    '\r[%s] %03d°%s %03d°%s T-%03d LOS %s 100%%' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90), self.t_1 - self.t_c, datetime.fromtimestamp(self.t_0).strftime('%Y-%m-%d %H:%M')))
                self.state = self.STATE_RESET
                return True

            t = (self.t_c - self.t_0) / self.t_d
            steps = self.move(t)
            sys.stdout.write(
                '\r[%s] %03d°%s %03d°%s T-%03d LOS %s %03d%% (%d/%d)' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 90), self.t_1 - self.t_c, datetime.fromtimestamp(self.t_0).strftime('%Y-%m-%d %H:%M'), t * 100, steps.get('az'), steps.get('el')))

        if self.STATE_RESET == self.state:
            self.move(0)
            sys.stdout.write('\n[%s] %03d°%s %03d°%s' % (self.state, self.az_c, bearing(self.az_c, 360), self.el_c, bearing(self.el_c, 360)))
            self.state = self.STATE_DONE
            sys.stdout.write('\n')
            return False

        return True

    def move(self, t):
        # target
        delta = (self.az_1 - self.az_0 + 180) % 360 - 180
        self.az_t = (self.az_0 + (delta * t)) % 360
        self.el_t = 4 * self.el_1 * t * (1 - t)
        steps = {'az': (self.az_t - self.az_c) * self.az_d, 'el': (self.el_t - self.el_c) * self.el_d}

        # azimuth
        if self.az_m:
            self.az_m.TurnStep('forward', abs(steps.get('az')), .001)

        # elevation
        if self.el_m:
            self.el_m.TurnStep('forward', abs(steps.get('el')), .001)

        # update
        self.az_c = self.az_t
        self.el_c = self.el_t

        return steps


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
            value = input('[PARAM] ' + label + ' ')
            if value == '':
                return None

            return callback(value)

        except ValueError:
            pass


def main():
    tracker = Tracker()

    if 'y' != input('[PARAM] execute         [y/n]: ').lower():
        sys.stdout.write('aborted\n')
        sys.stdout.flush()
        return

    tracker.execute()


if __name__ == '__main__':
    main()
