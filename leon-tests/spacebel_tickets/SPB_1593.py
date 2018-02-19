# unpack allows input buffer to be longer than needed
import ustruct as struct
s = struct.pack('ii', 1, 2)
try:
    print(struct.unpack('i', s))
except:
    print('Exception')
