# check special cases of pow that shouldn't raise

import math

# these cases would originally raise ValueError
print(math.pow(0.9, -float('inf')))
print(math.pow(-0.9, -float('inf')))

print(math.pow(0.9, float('inf')))
print(math.pow(-0.9, float('inf')))

print(math.pow(0.0, float('inf')))
print(math.pow(1.0, 0.0))
print(math.pow(-1.0, 0.0))
print(math.pow(1.1, float('inf')))
print(math.pow(-1.1, float('inf')))
