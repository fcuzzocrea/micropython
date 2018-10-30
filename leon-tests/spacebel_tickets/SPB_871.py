# test passing in out-of-bounds bytes to bytearray

try:
    bytearray([-1])
except ValueError:
    print('ValueError')

try:
    bytearray([256])
except ValueError:
    print('ValueError')

try:
    bytearray(1)[0] = -1
except ValueError:
    print('ValueError')

try:
    bytearray(1)[0] = 256
except ValueError:
    print('ValueError')

try:
    bytearray().append(-1)
except ValueError:
    print('ValueError')

try:
    bytearray().append(256)
except ValueError:
    print('ValueError')

# bigint values should be ok if they are in bounds
big = 1 << 70
big_zero = big - big
big_one = big >> 70
print(bytearray([big_zero, big_one]))

try:
    bytearray([-big_one])
except ValueError:
    print('ValueError')
