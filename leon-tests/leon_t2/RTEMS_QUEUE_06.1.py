# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_06

import micropython
import rtems

mq = rtems.queue.create('tstq', 10)

# below tests should not need the heap
micropython.heap_lock()

mq.send('hello leon')
