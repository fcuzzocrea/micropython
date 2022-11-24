from ulab import numpy as np
from ulab import utils


def endian_swap_16(a):
    for i in range(0, len(a), 2):
        a[i], a[i + 1] = a[i + 1], a[i]
    return a


def endian_swap_32(a):
    for i in range(0, len(a), 4):
        a[i], a[i + 1], a[i + 2], a[i + 3] = a[i + 3], a[i + 2], a[i + 1], a[i]
    return a


a = endian_swap_16(bytearray([1, 0, 0, 1, 0, 255, 255, 255]))
print(utils.from_uint16_buffer(a))
a = endian_swap_16(bytearray([1, 0, 0, 1, 0, 255, 255, 255]))
print(utils.from_int16_buffer(a))

a = endian_swap_32(bytearray([1, 0, 0, 1, 0, 255, 255, 255]))
print(utils.from_uint32_buffer(a))
a = endian_swap_32(bytearray([1, 0, 0, 1, 0, 255, 255, 255]))
print(utils.from_int32_buffer(a))

a = endian_swap_32(bytearray([1, 0, 0, 1, 0, 0, 255, 255]))
print(utils.from_uint32_buffer(a))
a = endian_swap_32(bytearray([1, 0, 0, 1, 0, 0, 255, 255]))
print(utils.from_int32_buffer(a))

a = endian_swap_32(bytearray([1, 0, 0, 0, 0, 0, 0, 1]))
print(utils.from_uint32_buffer(a))
a = endian_swap_32(bytearray([1, 0, 0, 0, 0, 0, 0, 1]))
print(utils.from_int32_buffer(a))
