# test that math.pi and math.e constants are exact

import math, array

print(math.pi, bytes(array.array('d', [math.pi])))
print(math.e, bytes(array.array('d', [math.e])))
