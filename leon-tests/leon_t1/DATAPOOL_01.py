# Test for MicroPython port to LEON
# Test ID: DATAPOOL_01

import datapool

# test ident when datapool does not exist
try:
    dp = datapool.ident('DP1')
except OSError as er:
    print('OSError:', er)

# create a datapool
dp = datapool.create('DP1')

# test ident when datapool exists
print(repr(datapool.ident('DP1')) == repr(dp))

# test create when datapool exists
try:
    dp = datapool.create('DP1')
except OSError as er:
    print('OSError:', er)
