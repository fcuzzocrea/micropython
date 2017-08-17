# membership op was too permissive for strings

try:
    None in ''
except TypeError:
    print('TypeError')

try:
    None not in ''
except TypeError:
    print('TypeError')

try:
    1 in '123'
except TypeError:
    print('TypeError')

try:
    1 not in '123'
except TypeError:
    print('TypeError')
