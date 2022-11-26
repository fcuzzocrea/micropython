# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_05

import micropython
import time
import rtems

script_id = '[T%u]' % rtems.script_id()

time.sleep(1)
mq = rtems.queue.ident('tstq', 0)
buf = bytearray(10)
result = None

# below tests should not need the heap
micropython.heap_lock()

result = mq.receive(buf)

time.sleep(2) # wait for script 1 to receive

print(script_id, 'queue receive:', result)
print(script_id, 'queue buf:', buf)
buf[9] = ord('3')
print(script_id, 'queue broadcast:', mq.broadcast(buf))
