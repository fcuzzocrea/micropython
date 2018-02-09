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

# these shouldn't raise an exception
print(0.0 ** float('-inf'))
print(pow(0.0, float('-inf')))
