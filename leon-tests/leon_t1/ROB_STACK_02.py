# Test for MicroPython port to LEON
# Test ID: ROB_STACK_02

def f(x):
    # 2 * 24 = stack size of 48 objects = 384 bytes
    l = [x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,
        x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,]
    if x:
        return len(l) + f(x - 1)
    return len(l)

for i in range(25):
    try:
        print(i, f(i))
    except RuntimeError:
        print(i, '(stack overflow)')
