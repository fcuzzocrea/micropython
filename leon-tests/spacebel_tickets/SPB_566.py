# VM halts on call to rsplit
# these calls gave nil results, or MemoryError exceptions

print('Hello world !'.rsplit(' ', -1))
print('Hello world !'.rsplit(' ', -2))
