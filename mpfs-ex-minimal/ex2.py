# Example script #2

import micropython
import time
import math
import rtems

# test importing a pure Python script
import ex3
print(ex3.data)
ex3.foo(1)

def main():
    tod = 6 * [0]

    micropython.heap_lock()

    print('MicroPython script 2')
    print('RTEMS script id:', rtems.script_id())

    time.localtime_into(tod)
    print('datetime:', tod)

    for i in range(1, 10):
        print('sin(pi / ', i, ') = ', round(math.sin(math.pi / i), 4), sep='')

main()
