# Test for MicroPython port to LEON
# Test ID: NOHEAP_02

i = None
tpl = (1, 2, 3, 4)

import micropython
micropython.heap_lock()

# print
print(())
print(tuple())
print(tpl)

# load
print(tpl[0])

# bool
print(bool(()))
print(bool(tpl))

# hash
print(bool(()))
print(bool(tpl))

# len
print(len(()))
print(len(tpl))

# cmp ops
print(tpl == tpl)
print(tpl == ())
print(tpl < ())
print(tpl > ())
print(tpl >= tpl)
print(tpl <= tpl)

# count
print(tpl.count(0), tpl.count(1))

# index
print(tpl.index(1))

# iterate
for i in tpl:
    print(i)
