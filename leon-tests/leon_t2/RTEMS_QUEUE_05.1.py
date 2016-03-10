# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_05

import micropython
import time
import rtems

script_id = '[T%u]' % rtems.script_id()

mq = rtems.queue.create('tstq', 10)
buf = bytearray(10)

# below tests should not need the heap
micropython.heap_lock()

print(script_id, 'queue broadcast:', mq.broadcast('broadcast1'))
print(script_id, 'queue pending:', mq.get_number_pending())
time.sleep(2) # wait for script 2 to connect to queue
print(script_id, 'queue broadcast:', mq.broadcast('broadcast2'))
print(script_id, 'queue receive:', mq.receive(buf))
print(script_id, 'queue buf:', buf)
