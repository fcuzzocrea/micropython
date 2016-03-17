# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_03

import micropython
import rtems

script_id = '[T%u]' % rtems.script_id()

mq = rtems.queue.create('tstq', 4, 10)

# below tests should not need the heap
micropython.heap_lock()

mq.send('normal msg')
mq.urgent('urgent msg')
print(script_id, 'queue pending:', mq.get_number_pending())
