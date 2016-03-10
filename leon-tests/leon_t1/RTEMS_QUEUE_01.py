# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_01

import micropython
from rtems import queue

try:
    q = queue.create('q', 10)
except ValueError as er:
    print(repr(er))

q = queue.create('qqqq', 10)
buf = bytearray(10)

micropython.heap_lock()

q.send('message')
print(q.get_number_pending())
print(q.receive(buf))
print(buf)

buf[0] = ord('M')
q.send(buf)
print(q.receive(buf))
print(buf)
