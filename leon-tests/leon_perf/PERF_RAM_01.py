# Test for MicroPython port to LEON
# Test ID: PERF_RAM_01

import micropython

def f1(x):
    micropython.mem_info()
    if x:
        f1(x - 1)
print('-- calling f1 --')
f1(4)

def f2(x, y):
    micropython.mem_info()
    if x:
        f2(x - 1, y)
print('-- calling f2 --')
f2(4, 4)
