# built-in dir() should list everything

class A:
    a = 1
    def f(self):
        pass

class B(A):
    b = 2
    def g(self):
        pass

print(sorted(n for n in dir(A()) if not n.startswith('_')))
print(sorted(n for n in dir(B()) if not n.startswith('_')))
