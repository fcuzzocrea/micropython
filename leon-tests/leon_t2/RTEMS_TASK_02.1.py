# Test for MicroPython port to LEON
# Test ID: RTEMS_TASK_02

import micropython
import rtems
micropython.heap_lock()

print('MicroPython script')
print('task 1, script id', rtems.script_id())
