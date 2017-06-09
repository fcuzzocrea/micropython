# invalid float literals
# these particular numbers gave ValueError at some point

print(1e200)
print(1.e200)
print(1.0e200)
print(1e20)
print(1.e20)
print(1.0e20)

print(1e-200)
print(1.e-200)
print(1.0e-200)
print(1e-20)
print(1.e-20)
print(1.0e-20)

print(10e199)
print(10.0e199)
print(10e-199)
print(10.0e-199)
