# Example script #2

import rtems

print('MicroPython script 2')
for i in range(100):
    print('%.4f' % (1 / (i + 1)))
    rtems.task.wake_after(35)
