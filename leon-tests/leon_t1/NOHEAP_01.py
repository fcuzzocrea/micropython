# Test for MicroPython port to LEON
# Test ID: NOHEAP_01

lst = [0, 1, 2, 3]
itr = range(4)
comp1 = (2 - i for i in range(4))
comp2 = (2 - i for i in range(4))
comp3 = (2 - i for i in range(4))
comp4 = (2 - i for i in range(4))
comp5 = (2 - i for i in range(4))
comp6 = (2 - i for i in range(4))
class A:
    x = 'x value'
a = A()

import micropython
micropython.heap_lock()

print(abs(-1))
print(all(lst))
print(all(itr))
print(all(comp1))
print(any(lst))
print(any(itr))
print(any(comp2))
print(callable(abs), callable(lst))
print(getattr(a, 'x'))
print('__name__' in globals())
print(hasattr(lst, 'sort'))
print(hash(123))
print(isinstance(1, int))
print(issubclass(int, int))
print(len(lst))
print(len(itr))
print('__name__' in locals())
print(max(3, 2, 4))
print(max(lst))
print(max(itr))
print(max(comp4))
print(min(3, 2, 4))
print(min(lst))
print(min(itr))
print(min(comp5))
print(ord('a'))
print(pow(2, 3))
print(sum(lst))
print(sum(itr))
print(sum(comp6))
