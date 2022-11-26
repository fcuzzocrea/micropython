# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
#
# SMP heap stress test, which allocate lots of objects (around 5MiB on the heap).
# Can be run as 1 or more instances at once.
# Based on micropython/tests/stress_heap.py.

import time
import rtems

# Simple lock with context manager, based on RTEMS semaphore.
class Lock:
    def __init__(self):
        if rtems.script_id() == 0:
            self.sem = rtems.sem.create("LOCK")
        else:
            while True:
                try:
                    self.sem = rtems.sem.ident("LOCK")
                    break
                except OSError:
                    pass
                time.sleep(0.1)

    def delete(self):
        if rtems.script_id() == 9:
            time.sleep(5)
            self.sem.delete()

    def __enter__(self):
        self.sem.obtain()

    def __exit__(self, a, b, c):
        self.sem.release()


def last(l):
    return l[-1]


def test(n):
    # Allocate a bytearray and fill it.
    data = bytearray(i for i in range(256))

    # Run a loop which allocates a small list and uses it each iteration.
    lst = 8 * [0]
    sum = 0
    for i in range(n):
        sum += last(lst)
        lst = [0, 0, 0, 0, 0, 0, 0, i + 1]

    # Check that the bytearray still has the right data.
    for i, b in enumerate(data):
        assert i == b

    # Print the result of the loop and indicate we are finished.
    lock = Lock()
    with lock:
        print(sum, lst[-1])
    lock.delete()


test(10000)
