# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_02

import micropython
import rtems

script_id = '[T%u]' % rtems.script_id()

mq = rtems.queue.create('tstq', 10)

# below tests should not need the heap
micropython.heap_lock()

mq.send('hello leon')
print(script_id, 'queue pending:', mq.get_number_pending())
