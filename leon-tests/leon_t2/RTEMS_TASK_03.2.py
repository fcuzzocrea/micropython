# Test for MicroPython port to LEON
# Test ID: RTEMS_TASK_03

import micropython
import rtems
i = None

# below tests should not need the heap
micropython.heap_lock()

rtems.task.wake_after(1000)
for i in range(10):
    print('script', rtems.script_id(), 'iter', i)
    rtems.task.wake_after(2000)
