# Exception raised when comparing double with other type
# VM was too eager to optimise float binary ops

print(0.0 is None)
print(0.0 == None)
print(0.0 != None)
print(None is 0.0)
