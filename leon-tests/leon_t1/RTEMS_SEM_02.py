# Test for MicroPython port to LEON
# Test ID: RTEMS_SEM_02

import micropython
import rtems
from rtems import sem

# create semaphore with 2 units
s = sem.create('ssss', 2)

er = None # global variable needed for try/except below

micropython.heap_lock()

# simple obtain/release
s.obtain()
print('have sem 1')
s.release()

# obtain twice
s.obtain()
print('have sem 1')
s.obtain()
print('have sem 2')
s.release()
s.release()

# obtain twice, two times
s.obtain()
print('have sem 1')
s.obtain()
print('have sem 2')
s.release()
s.obtain()
print('have sem 2')
s.release()
s.release()

# obtain three times, third should fail
s.obtain()
print('have sem 1')
s.obtain()
print('have sem 2')
try:
    s.obtain(option=rtems.NO_WAIT)
except OSError as er:
    print('OSError:', er)
s.release()
s.release()
