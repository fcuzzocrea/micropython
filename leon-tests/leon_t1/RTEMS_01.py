# Test for MicroPython port to LEON
# Test ID: RTEMS_01

import micropython
import rtems

# below tests should not need the heap
micropython.heap_lock()

print('MicroPython script')
print('script id:', rtems.script_id())
