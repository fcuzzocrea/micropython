# Test for MicroPython port to LEON
# Test ID: DATAPOOL_05

import micropython
import rtems
import datapool

script_id = '[T%u]' % rtems.script_id()

dp = datapool.create('DP')
buf = bytearray(10)

# below tests should not need the heap
micropython.heap_lock()

dp.set_buf(10, 'hello leon')

dp.get_buf(10, buf)
print(script_id, 'buf:', buf)
