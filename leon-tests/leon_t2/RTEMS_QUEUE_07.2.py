# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_07

import micropython
import time
import rtems

script_id = '[T%u]' % rtems.script_id()

time.sleep(1)
mq_mosi = rtems.queue.ident('mosi', 0)
mq_miso = rtems.queue.ident('miso', 0)
buf = bytearray(5)

# below tests should not need the heap
micropython.heap_lock()

print(script_id, 'queue receive:', mq_mosi.receive(buf))
print(script_id, 'queue buf:', buf)
print(script_id, 'queue receive:', mq_mosi.receive(buf))
print(script_id, 'queue buf:', buf)
mq_miso.send('out2a')
mq_miso.send('out2b')
