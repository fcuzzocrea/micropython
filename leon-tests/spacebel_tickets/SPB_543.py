# Invalid struct size

import ustruct as struct

try:
    struct.calcsize('=IL')
except ValueError:
    print('ValueError')

try:
    struct.pack_into('=IL', bytearray(16), 0, 0)
except ValueError:
    print('ValueError')
