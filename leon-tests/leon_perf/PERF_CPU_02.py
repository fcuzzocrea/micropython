# Test for MicroPython port to LEON
# Test ID: PERF_CPU_02

import micropython
import time

# looping
def fib_loop(n):
    a, b = 1, 1
    for i in range(n - 1):
        a, b = b, a+b
    return a
 
# recursion
def fib_recurse(n):
    if n == 1 or n == 2:
        return 1
    return fib_recurse(n - 1) + fib_recurse(n - 2)
 
def time_it(f):
    start = time.time()
    for i in range(1, 16):
        f(i)
    end = time.time()
    print('time:', end - start)

def main():
    micropython.heap_lock()
    time_it(fib_loop)
    time_it(fib_recurse)

main()
