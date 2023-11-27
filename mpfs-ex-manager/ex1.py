# Example script #1

import rtems

print('MicroPython script 1')
for i in range(100):
    print(i)
    rtems.task.wake_after(35)
