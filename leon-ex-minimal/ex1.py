# Example script #1

import micropython
import time
import rtems

class MovingAverage:
    def __init__(self, n):
        self.values = [0] * n

    def add_value(self, value):
        for i in range(len(self.values) - 1):
            self.values[i + 1] = self.values[i]
        self.values[0] = value

    def get_average(self):
        total = 0
        for i in range(len(self.values)):
            total += self.values[i]
        return total / len(self.values)

class Sensor:
    def __init__(self, name, source):
        self.name = name
        self.source = source
        self.filter = MovingAverage(5)

    def sample(self):
        self.filter.add_value(self.source())
        return self.filter.get_average()

class Indicator:
    def __init__(self, name):
        self.name = name
        self.state = False

    def toggle(self):
        self.state = not self.state

def create():
    # this is not very random!
    def random(state=[1234]):
        state[0] = (0x31a9 * state[0] + 0xce34) & 0xffff
        return state[0]

    sensors = {
        "random1":Sensor("random1", lambda: random() % 100),
        "random2":Sensor("random2", lambda: random() % 100),
    }

    indicators = {
        "indicator1":Indicator("indicator1"),
        "indicator2":Indicator("indicator2"),
    }

    return sensors, indicators

def loop(sensors, indicators):
    for i in range(10):
        s1 = sensors["random1"].sample()
        s2 = sensors["random2"].sample()
        if s1 + s2 > 50:
            indicators["indicator1"].toggle()
        else:
            indicators["indicator2"].toggle()
        print('iter', i, 'time', time.time(), int(s1), int(s2),
            indicators["indicator1"].state, indicators["indicator2"].state)
        time.sleep(0.5)

def main():
    sensors, indicators = create()
    micropython.heap_lock()
    print('MicroPython script 1')
    print('RTEMS script id:', rtems.script_id())
    loop(sensors, indicators)

main()
