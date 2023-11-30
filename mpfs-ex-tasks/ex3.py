import micropython
import time
import rtems

for i in range(20):
    print('MicroPython task', rtems.script_id(), 'iter', i)
    time.sleep(2.4)
