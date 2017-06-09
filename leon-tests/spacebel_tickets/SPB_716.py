# No exception raised for 0 with a negative power

try:
    (-0)**-1
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    (-0.0)**-1.0
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    pow(-0.0, -1.0)
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    0**-1
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    0.0**-1.0
except ZeroDivisionError:
    print('ZeroDivisionError')

try:
    pow(0.0, -1.0)
except ZeroDivisionError:
    print('ZeroDivisionError')
