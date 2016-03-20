# Test for MicroPython port to LEON
# Test ID: DATAPOOL_02

import datapool

dp = datapool.create('DP01')

# set_u32/get_u32 basic
dp.set_u32(100, 0xabcdffff)
print(hex(dp.get_u32(100)))

# set_u32/get_u32 basic
dp.set_u32(100, 0xffffabcd)
print(hex(dp.get_u32(100)))

# set_u32 entire bit range
for i in range(32):
    dp.set_u32(100 + i, 1 << i)

# should be able to access datapool with its heap locked
datapool.heap_lock()

# get_u32 entire bit range
for i in range(32):
    print(hex(dp.get_u32(100 + i)))

# get non-existent entry
try:
    dp.get_u32(200)
except KeyError as er:
    print('KeyError:', er)
