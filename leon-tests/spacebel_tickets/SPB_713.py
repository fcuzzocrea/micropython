# round function returns 0 insted of raising ValueError (for NAN value)

try:
    round(float('nan'))
except ValueError:
    print('ValueError')
