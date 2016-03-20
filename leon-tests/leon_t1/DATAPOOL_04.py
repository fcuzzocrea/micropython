# Test for MicroPython port to LEON
# Test ID: DATAPOOL_04

import datapool

dp = datapool.create('DP1')

# set_buf/get_buf basic
dp.set_buf(100, b'1234')
buf = bytearray(4)
dp.get_buf(100, buf)
print(buf)

# get non-existent entry
try:
    dp.get_buf(200, bytearray(4))
except KeyError as er:
    print('KeyError:', er)

# get to wrong size buffer
try:
    dp.get_buf(100, bytearray(5))
except ValueError as er:
    print('ValueError:', er)
