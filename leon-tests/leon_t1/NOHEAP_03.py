# Test for MicroPython port to LEON
# Test ID: NOHEAPP_03

empty_list = []
lst = [2, 1, 4, 3]

def negation(x):
    return -x

import micropython
micropython.heap_lock()

# list print
print(lst)

# load
print(lst[0])

# store
lst[0] = 0
print(lst)
lst[0] = 2

# modification
for i in range(len(lst)):
    lst[i] += 10
print(lst)

# bool
print(bool(empty_list))
print(bool(lst))

# len
print(len(empty_list))
print(len(lst))

# cmp ops
print(lst == lst)
print(lst == empty_list)
print(lst < empty_list)
print(lst > empty_list)
print(lst >= lst)
print(lst <= lst)

# count
print(lst.count(10), lst.count(11))

# index
print(lst.index(11))

# reverse
lst.reverse()
print(lst)

# sort
lst.sort()
print(lst)

# sort with key function
lst.sort(key=negation)
print(lst)

# iterate
for i in lst:
    print(i)
