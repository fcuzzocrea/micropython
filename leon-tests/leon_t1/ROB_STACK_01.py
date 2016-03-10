# Test for MicroPython port to LEON
# Test ID: ROB_STACK_01

def f(x):
    if x:
        return x + f(x - 1)
    return 0

for i in range(20):
    try:
        print(i, f(i))
    except RuntimeError:
        print(i, '(stack overflow)')
