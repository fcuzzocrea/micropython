# relative import not implemented (or gives a KeyError if it is)
try:
    __import__('xyz', None, None, None, 1)
except (KeyError, NotImplementedError):
    print('NotImplementedError')

# coverage for __repl_print__
__repl_print__(123)

# coverage for mp_identity function
import micropython
c = micropython.const
print(c(123))

# test mp_load_name when it gets the value from locals()
class A:
    x = 1
    y = x + 1
print(A.x, A.y)

# test overflow of 47-bit small-int in nan-boxing config
i = -0x3fffffffffff
print(-(i - 1))

# test pystack_use()
import micropython
print(type(micropython.pystack_use()))
