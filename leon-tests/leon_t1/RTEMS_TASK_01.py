# Test for MicroPython port to LEON
# Test ID: RTEMS_TASK_01

import micropython
import rtems
i = None

# below tests should not need the heap
micropython.heap_lock()

for i in range(10):
    print(i)
    rtems.task.wake_after(10)
