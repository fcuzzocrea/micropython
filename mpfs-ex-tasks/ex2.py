import micropython
import time
import rtems

def fib_loop(n):
    a, b = 1, 1
    for i in range(n - 1):
        a, b = b, a + b
    return a
 
for i in range(1, 30):
    print('MicroPython task', rtems.script_id(), 'fib_loop(%u) = %u' % (i, fib_loop(i)))
    time.sleep(1)
