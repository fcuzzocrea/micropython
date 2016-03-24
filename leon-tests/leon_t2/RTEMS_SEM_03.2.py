# Test for MicroPython port to LEON
# Test ID: RTEMS_SEM_03

import micropython
import time
import rtems

script_id = '[T%u]' % rtems.script_id()

time.sleep(0.5)
s = rtems.sem.ident('ssss')

i = None # global i used in for loop below

micropython.heap_lock()

for i in range(4):
    s.obtain()
    print(script_id, 'obtained sem')
    time.sleep(0.5)
    print(script_id, 'releasing sem')
    s.release()
