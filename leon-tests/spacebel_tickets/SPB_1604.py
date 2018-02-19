from array import array

b1 = array('b', [0, 1])
b2 = array('b', [0, 1])
b3 = array('b', [1, 2])
f1 = array('f', [0, 1])

print(b1 == b2)
print(b1 == b3)

try:
    print(b1 == f1)
except NotImplementedError:
    print('NotImplementedError')
