import micropython
import time
import rtems

def fib_recurse(n):
    if n == 1 or n == 2:
        return 1
    return fib_recurse(n - 1) + fib_recurse(n - 2)
 
for i in range(1, 30):
    try:
        f = fib_recurse(i)
    except RuntimeError as e:
        f = '(stack overflow)'
    print('MicroPython task', rtems.script_id(), 'fib_recurse(%u) = %s' % (i, f))
    time.sleep(1)
