# Test for MicroPython port to LEON
# Test ID: MODMEM_01

import array
import mem

# determine memory address based on platform
import sys
if sys.platform == "leon4":
    mem_base = 0x00100000
else:
    mem_base = 0x40100000

# write_u32
mem.write_u32(mem_base, 0xabcd1234)
mem.write_u32(mem_base + 4, 0xffffffff)

# write_buf
mem.write_buf(mem_base + 8, b'123456789')

# read_u32
for i in range(0, 16, 4):
    print(hex(mem.read_u32(mem_base + i)))

# read_buf into a bytearray
buf = bytearray(9)
mem.read_buf(mem_base, buf)
print(buf)

# read_buf into an array.array
ar = array.array('L', 2*[0])
mem.read_buf(mem_base, ar)
for x in ar:
    print(hex(x))
