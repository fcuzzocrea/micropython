# Test for MicroPython port to LEON
# Test ID: DATAPOOL_03

import datapool

dp = datapool.create('DP01')

# set_double/get_double basic
dp.set_double(100, 1.23e50)
print('%.3g' % dp.get_double(100))

# set_double/get_double basic
dp.set_double(100, 1.23e-50)
print('%.3g' % dp.get_double(100))

# set_double/get_double from int
dp.set_double(100, 12)
print('%.2f' % dp.get_double(100))

# set_double range
for i in range(10):
    dp.set_double(100 + i, 10.0**i)

# should be able to access datapool with its heap locked
datapool.heap_lock()

# get_double range
for i in range(10):
    print('%.2f' % dp.get_double(100 + i))

# get non-existent entry
try:
    dp.get_double(200)
except KeyError as er:
    print('KeyError:', er)
