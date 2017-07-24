# pow with 3rd arg used naive impementation

print(pow(5, 2000000, 5))

try:
    pow(2., 2000, 2)
except TypeError:
    print('TypeError')
