# range accepted 0 for 3rd argument

try:
    range(1, 2, 0)
except ValueError:
    print('ValueError')
