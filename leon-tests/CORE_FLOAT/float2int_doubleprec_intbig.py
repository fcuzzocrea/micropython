# check cases converting float to int, requiring double precision float

try:
    import ustruct as struct
except:
    import struct

# This case occurs with time.time() values
print(int(1418774543.))
print("%d" % 1418774543.)

testpass = True
p2_rng = 1024
for i in range(0,p2_rng):
    bitcnt = len(bin(int(2.**i))) - 3;
    if i != bitcnt:
        print('fail: 2**%u was %u bits long' % (i, bitcnt));
        testpass = False
print("power of  2 test: %s" % (testpass and 'passed' or 'failed'))

testpass = True
p10_rng = 23
for i in range(0,p10_rng):
    digcnt = len(str(int(10.**i))) - 1;
    if i != digcnt:
        print('fail: 10**%u was %u digits long' % (i, digcnt));
        testpass = False
print("power of 10 test: %s" % (testpass and 'passed' or 'failed'))

def fp2int_test(num, name, should_fail):
    try:
        x = int(num)
        passed = ~should_fail
    except:
        passed = should_fail
    print('%s: %s' % (name, passed and 'passed' or 'failed'))

fp2int_test(-1.9999999999999981*2.**1023., 'large neg', False)
fp2int_test(1.9999999999999981*2.**1023., 'large pos', False)

fp2int_test(float('inf'), 'inf test', True)
fp2int_test(float('nan'), 'NaN test', True)

# test numbers < 1 (this used to fail; see issue #1044)
fp2int_test(0.0001, 'small num', False)
struct.pack('I', int(1/2))
