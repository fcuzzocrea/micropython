# Test for MicroPython port to LEON
# Test ID: RTEMS_QUEUE_01

import micropython
import rtems
from rtems import queue

try:
    q = queue.create('q', 4, 10)
except ValueError as er:
    print(repr(er))

q = queue.create('qqqq', 4, 10)
buf = bytearray(10)

er = None # global variable needed for try/except below

micropython.heap_lock()

q.send('message')
print(q.get_number_pending())
print(q.receive(buf))
print(buf)

buf[0] = ord('M')
q.send(buf)
print(q.receive(buf))
print(buf)

# test non-blocking receive
try:
    q.receive(buf, rtems.NO_WAIT)
except OSError as er:
    print('OSError:', er)

# test blocking receive with a small timeout
try:
    q.receive(buf, rtems.WAIT, 100)
except OSError as er:
    print('OSError:', er)

# fill queue
q.send(buf)
q.send(buf)
q.send(buf)
q.send(buf)

# test non-blocking send (default behaviour)
try:
    q.send(buf, rtems.NO_WAIT)
except OSError as er:
    print('OSError:', er)

# test blocking send with a small timeout
try:
    q.send(buf, rtems.WAIT, 100)
except OSError as er:
    print('OSError:', er)
