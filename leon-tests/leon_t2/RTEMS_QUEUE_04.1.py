# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_04

import micropython
import rtems

script_id = '[T%u]' % rtems.script_id()

mq = rtems.queue.create('tstq', 4, 10)

# below tests should not need the heap
micropython.heap_lock()

print(script_id, 'queue pending:', mq.get_number_pending())
mq.send('data 1')
print(script_id, 'queue pending:', mq.get_number_pending())
mq.send('data 2')
print(script_id, 'queue pending:', mq.get_number_pending())
print(script_id, 'queue flush:', mq.flush())
print(script_id, 'queue pending:', mq.get_number_pending())
mq.send('hello leon')
print(script_id, 'queue pending:', mq.get_number_pending())
