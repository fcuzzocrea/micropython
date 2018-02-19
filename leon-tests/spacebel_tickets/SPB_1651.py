# test that small int range is 2**46-1

import micropython

def check(n):
    micropython.heap_lock()
    x = -(2**(n - 1)) * 2
    y = -(x + 1)
    print(x)
    print(y)
    micropython.heap_unlock()

check(43)
check(44)
check(45)
check(46)

try:
    check(47)
except MemoryError:
    print('MemoryError')
