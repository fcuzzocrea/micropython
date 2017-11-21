# changing dict while iterating
d = {1:1}
try:
    for i in d:
        d[i + 1] = i
except RuntimeError:
    print('RuntimeError')

# also test dict views
d = {1:1}
try:
    for i in d.keys():
        d[2] = 2
except RuntimeError:
    print('RuntimeError')

d = {1:1}
try:
    for i in d.values():
        d[2] = 2
except RuntimeError:
    print('RuntimeError')

d = {1:1}
try:
    for i in d.items():
        d[2] = 2
except RuntimeError:
    print('RuntimeError')

# also test set
s = {1}
try:
    for i in s:
        s.add(2)
except RuntimeError:
    print('RuntimeError')
