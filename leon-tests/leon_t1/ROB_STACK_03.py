# Test for MicroPython port to LEON
# Test ID: ROB_STACK_03

def f(x):
    # 4 * 24 = stack size of 96 objects = 768 bytes
    l = [x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,
        x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,
        x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,
        x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,]
    if x:
        return len(l) + f(x - 1)
    return len(l)

for i in range(10):
    try:
        print(i, f(i))
    except RuntimeError:
        print(i, '(stack overflow)')
