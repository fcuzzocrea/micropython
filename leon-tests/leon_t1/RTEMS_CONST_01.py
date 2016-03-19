# Test for MicroPython port to LEON
# Test ID: RTEMS_CONST_01

import rtems

# make sure constants have expected value
print(hex(rtems.DEFAULT_ATTRIBUTES))
print(hex(rtems.WAIT))
print(hex(rtems.NO_WAIT))
print(hex(rtems.SEARCH_ALL_NODES))
print(hex(rtems.SEARCH_OTHER_NODES))
print(hex(rtems.SEARCH_LOCAL_NODE))
