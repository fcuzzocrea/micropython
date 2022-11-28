# Test for MicroPython port to LEON
# Test ID: TASK_AFFINITY_02
# Test description: Do heavy processing (compute digits of pi) in multiple tasks,
#                   running on all available CPUs.  This tests setting affinity and
#                   concurrent execution.

import sys
import rtems

NUM_CPU = {"leon2": 1, "leon3": 2, "leon4": 4}[sys.platform]
NUM_TASKS = 8

if NUM_CPU > 1:
    # Run tasks on all CPUs.
    affinity = (1 << NUM_CPU) - 1

    rtems.task.set_affinity(affinity)
    if rtems.task.get_affinity() != affinity:
        print("Affinity not set:", rtems.task.get_affinity())

# This function is adapted from: https://github.com/python/pyperformance
# License: MIT
# Original header comment:
#   Calculating some of the digits of π.
#   This benchmark stresses big integer arithmetic.
#   Adapted from code on: http://benchmarksgame.alioth.debian.org/
def gen_pi_digits(n):
    def compose(a, b):
        aq, ar, as_, at = a
        bq, br, bs, bt = b
        return (aq * bq, aq * br + ar * bt, as_ * bq + at * bs, as_ * br + at * bt)

    def extract(z, j):
        q, r, s, t = z
        return (q * j + r) // (s * j + t)

    z = (1, 0, 0, 1)
    k = 1
    digs = []
    for _ in range(n):
        y = extract(z, 3)
        while y != extract(z, 4):
            z = compose(z, (k, 4 * k + 2, 0, 2 * k + 1))
            k += 1
            y = extract(z, 3)
        z = compose((10, -10 * y, 0, 1), z)
        digs.append(y)
    return digs


# Simple lock with context manager, based on RTEMS semaphore.
class Lock:
    def __init__(self):
        if rtems.script_id() == 0:
            self.sem = rtems.sem.create("LOCK")
            self.mq = rtems.queue.create("SYNC", count=NUM_TASKS, max_size=1)
        else:
            while True:
                try:
                    self.sem = rtems.sem.ident("LOCK")
                    self.mq = rtems.queue.ident("SYNC")
                    break
                except OSError:
                    pass

    def delete(self):
        if rtems.script_id() == 0:
            buf = bytearray(1)
            for _ in range(NUM_TASKS - 1):
                try:
                    self.mq.receive(buf, timeout=50000)
                except OSError:
                    break
            self.sem.delete()
            self.mq.delete()
        else:
            self.mq.send("X")

    def __enter__(self):
        self.sem.obtain()

    def __exit__(self, a, b, c):
        self.sem.release()


lock = Lock()
digits = gen_pi_digits(110)
output = "".join("%d" % dig for dig in digits)
output = output[0] + "." + output[1:]
with lock:
    print(output)
lock.delete()
