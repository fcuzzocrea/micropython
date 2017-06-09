# VM halts with invalid range

try:
    range(0, 4, 0)
except ValueError:
    print('ValueError')

