# Test for MicroPython port to LEON
# Test ID: NOHEAP_04

i = 0 # create global "i" for iterators to work
dct = {'one':1, 'two':2, 'three':3}
dct_keys = dct.keys()
dct_values = dct.values()
other_dict = {'one':11}
other_tuple = (('one', 1),)

import micropython
micropython.heap_lock()

# print
print(dct)

# len
print(len(dct))

# load
print(dct['two'])

# store
dct['two'] = 22
print(dct)

# iteration
for i in dct:
    print(i)
for i in dct_keys:
    print(i)
for i in dct_values:
    print(i)

# get
print(dct.get('three', 333))
print(dct.get('four', 444))

# setdefault
print(dct.setdefault('two', 222))

# update
dct.update(other_dict)
print(dct)
dct.update(other_tuple)
print(dct)
dct.update(one=11)
print(dct)
