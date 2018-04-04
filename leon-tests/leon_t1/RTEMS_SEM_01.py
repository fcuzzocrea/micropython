# Test for MicroPython port to LEON
# Test ID: RTEMS_SEM_01

import micropython
import rtems
from rtems import sem

# test invalid name
try:
    s = sem.create('s')
except ValueError as er:
    print(repr(er))

s = sem.create('ssss')

er = None # global variable needed for try/except below

# test printing the sem object
print(repr(s)[:9])

micropython.heap_lock()

# simple obtain/release
s.obtain()
print('have sem')
s.release()

# obtain with extra positional args
s.obtain(rtems.NO_WAIT, 0)
print('have sem')
s.release()

# obtain keyword args
s.obtain(option=rtems.NO_WAIT, timeout=0)
print('have sem')
s.release()

# obtain twice, second should fail
s.obtain()
print('have sem')
try:
    s.obtain(option=rtems.NO_WAIT)
except OSError as er:
    print('OSError:', er)
s.release()

# obtain twice with non-zero timeout, second should fail
s.obtain()
print('have sem')
try:
    s.obtain(option=rtems.NO_WAIT, timeout=10)
except OSError as er:
    print('OSError:', er)
s.release()
