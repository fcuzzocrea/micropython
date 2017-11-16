# no exception was raised for 0**-1

import math

try:
    0. ** -1.
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    math.pow(0., -1,)
except ValueError:
    print('ValueError')
