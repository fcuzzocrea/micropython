# Test for MicroPython port to LEON
# Test ID: NOHEAP_05

import math

# pre-create spots in global hash table
x = None
y = None
lst = [1.2, 0, 2, -3.4, 5.7]

import micropython
micropython.heap_lock()

# construct
print(1.2)
print(float(), float(1), float(1.2), float('1.2'), float('1.2e3'), float('1.2e-3'))

# for following tests
x = 1.3
y = 3.5

# print
print(x, y)

# unary ops
print(bool(0.0), bool(x))
print(+x, -x)

# binary arithmetic
print(x + 2, x + 2.0, x + y)
print(x - 2, x - 2.0, x - y)
print(x * 2, x * 2.0, x * y)
print(y // 2, y // 2.0, y // x)
print(x / 2, x / 2.0, round(x / y, 4))
print(x % 2, x % 2.0, round(x % y, 4))
print(round(x**2, 2), round(x**2.0, 2), round(x**y, 4))

# binary comparison
print(x > x, x > 2, x > 2.2, x > y)
print(x < x, x < 2, x < 2.2, x < y)
print(x == x, x == 2, x == 2.2, x == y)
print(x <= x, x <= 2, x <= 2.2, x <= y)
print(x >= x, x >= 2, x >= 2.2, x >= y)

# builtin functions
print(all(lst))
print(any(lst))
print(abs(x), abs(-1.2))
print(min(0.5, x, y), min(lst))
print(max(0.5, x, y), max(lst))
print(round(x), round(y))
print(round(x, 2), round(y, 2))
print(sum(lst))

# math functions
print(round(math.e, 2))
print(round(math.pi, 2))
print(round(math.sqrt(x), 2))
print(round(math.pow(x, y), 2))
print(round(math.exp(x), 2))
print(round(math.log(x), 2))
print(round(math.cos(x), 2))
print(round(math.sin(x), 2))
print(round(math.tan(x), 2))
print(round(math.acos(x / 10), 2))
print(round(math.asin(x / 10), 2))
print(round(math.atan(x / 10), 2))
print(round(math.atan2(x, y), 2))
print(math.ceil(x))
print(math.copysign(x, -1))
print(math.fabs(-x))
print(math.floor(x))
print(math.fmod(x, y))
print(math.ldexp(x, 2))
print(math.isfinite(x), math.isfinite(float('inf')))
print(math.isinf(x), math.isinf(float('inf')))
print(math.isnan(x), math.isinf(float('nan')))
print(math.trunc(x))
print(round(math.radians(x), 2))
print(round(math.degrees(x), 2))
