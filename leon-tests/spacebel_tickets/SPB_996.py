import math

try:
    math.fmod(1., 0.)
except ValueError:
    print('ValueError')

try:
    math.fmod(float('inf'), 1.)
except ValueError:
    print('ValueError')
