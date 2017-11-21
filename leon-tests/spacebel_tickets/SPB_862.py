import array
try:
    import struct
except:
    import ustruct as struct

try:
    print(struct.calcsize('Z'))
except Exception as er:
    print('ValueError', er)

store = array.array('b', b' ' * 100)
try:
    struct.pack_into('12345', store, 0)
except Exception as er:
    print('ValueError', er)
print(store)

try:
    print(struct.pack('14s42', 'spam and eggs'))
except Exception as er:
    print('ValueError', er)

try:
    print(struct.pack('iii', 3))
except Exception as er:
    print('ValueError', er)

try:
    print(struct.pack('i', 3, 3, 3))
except Exception as er:
    print('ValueError', er)

try:
    print(struct.unpack('d', b'flap'))
except Exception as er:
    print('ValueError', er)

# running out of args during a repeat specifier
try:
    print(struct.pack('3i', 3, 3))
except Exception as er:
    print('ValueError', er)
