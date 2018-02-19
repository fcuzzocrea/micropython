# negative args to bytes/bytearray

try:
    bytes(-1)
except:
    print('Exception')
try:
    bytes(-2)
except:
    print('Exception')
try:
    bytes(-3)
except:
    print('Exception')
try:
    bytearray(-1)
except:
    print('Exception')
try:
    bytearray(-2)
except:
    print('Exception')
try:
    bytearray(-3)
except:
    print('Exception')
