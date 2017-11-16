try:
    set((1, 2)) - (1,)
except TypeError:
    print('TypeError')

try:
    set("hello") - "world"
except TypeError:
    print('TypeError')

try:
    set("hello") & "world"
except TypeError:
    print('TypeError')
