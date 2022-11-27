# Test for MicroPython port to LEON
# Test ID: RTEMS_TASK_02

import micropython
import rtems

# below tests should not need the heap
micropython.heap_lock()

# wait for task 1 to run
rtems.task.wake_after(200)

print('MicroPython script')
print('task 2, script id', rtems.script_id())
