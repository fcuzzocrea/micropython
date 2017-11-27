# locals() is not supported in all contexts
# here we test those contexts where it's not supported

def f():
    locals()
try:
    f()
except NotImplementedError:
    print('NotImplementedError')

try:
    (lambda: locals())()
except NotImplementedError:
    print('NotImplementedError')

try:
    [locals() for x in range(1)]
except NotImplementedError:
    print('NotImplementedError')

def f():
    locals()
    yield 1
try:
    next(f())
except NotImplementedError:
    print('NotImplementedError')
