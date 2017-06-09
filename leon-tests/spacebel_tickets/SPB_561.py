# Erroneous module results for MPZ integers

x = 1000000000000000000000000000000000000
print(x % -x)
print(divmod(x, -x))
print((-x) % x)
print(divmod(-x, x))
