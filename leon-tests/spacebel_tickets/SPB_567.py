# VM halts on string formatting
# previously, this would crash

try:
    print('%()' % 1)
except TypeError:
    print('TypeError')
