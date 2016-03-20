# Test for MicroPython port to LEON
# Test ID: DATAPOOL_05

import micropython
import time
import rtems
import datapool

script_id = '[T%u]' % rtems.script_id()

time.sleep(1)
dp = datapool.ident('DP')
buf = bytearray(10)

# below tests should not need the heap
micropython.heap_lock()

dp.get_buf(10, buf)
print(script_id, 'buf:', buf)
