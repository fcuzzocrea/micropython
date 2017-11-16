import math

print(type((-1)**2.3))

try:
    math.pow(-1, 2.3)
except ValueError:
    print('ValueError')
