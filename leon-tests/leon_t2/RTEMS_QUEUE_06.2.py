# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_06

import micropython
import time
import rtems

script_id = '[T%u]' % rtems.script_id()

time.sleep(1)
mq = rtems.queue.ident('tstq', 0)
buf = bytearray(10)
e = None

# below tests should not need the heap
micropython.heap_lock()

print(script_id, 'queue receive:', mq.receive(buf))
print(script_id, 'queue buf:', buf)
mq.delete()
try:
    mq.send('test')
except OSError as e:
    print(script_id, 'queue error:', e)
