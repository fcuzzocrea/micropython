# struct.pack function accepted invalid format specifier

import ustruct
try:
    ustruct.pack('1')
except ValueError:
    print('ValueError')
