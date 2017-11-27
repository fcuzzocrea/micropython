# containment acting on a subclass of a native type

class D(dict):
    pass

print("a" in D())
print(1 in D())
print(1.0 in D())

print(D({"a":123})["a"])
