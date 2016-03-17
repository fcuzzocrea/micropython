# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_07

import micropython
import rtems

script_id = '[T%u]' % rtems.script_id()

mq_mosi = rtems.queue.create('mosi', 4, 5)
mq_miso = rtems.queue.create('miso', 4, 5)
buf = bytearray(5)

# below tests should not need the heap
micropython.heap_lock()

mq_mosi.send('out1a')
mq_mosi.send('out1b')
print(script_id, 'queue receive:', mq_miso.receive(buf))
print(script_id, 'queue buf:', buf)
print(script_id, 'queue receive:', mq_miso.receive(buf))
print(script_id, 'queue buf:', buf)
