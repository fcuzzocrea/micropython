# Test for MicroPython port to LEON
# Test ID: ROB_HEAP_01

def create_bufs(n, sz):
    bufs = []
    for i in range(n):
        bufs.append(bytearray(sz))
    bufs = None

print('start')
for iter in range(5):
    print(iter)
    for sz in range(2, 65, 2):
        create_bufs(150, sz)
