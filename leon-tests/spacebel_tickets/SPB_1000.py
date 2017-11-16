import math

try:
    math.asin(2.)
except ValueError:
    print('ValueError')

try:
    math.acos(2.)
except ValueError:
    print('ValueError')

try:
    math.asin(float('inf'))
except ValueError:
    print('ValueError')
