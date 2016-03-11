# Test for MicroPython port to LEON
# Test ID: MODMEM_01

import array
import mem

# write_u32
mem.write_u32(0x40100000, 0xabcd1234)
mem.write_u32(0x40100004, 0xffffffff)

# write_buf
mem.write_buf(0x40100008, b'123456789')

# read_u32
for i in range(0, 16, 4):
    print(hex(mem.read_u32(0x40100000 + i)))

# read_buf into a bytearray
buf = bytearray(9)
mem.read_buf(0x40100000, buf)
print(buf)

# read_buf into an array.array
ar = array.array('L', 2*[0])
mem.read_buf(0x40100000, ar)
for x in ar:
    print(hex(x))
