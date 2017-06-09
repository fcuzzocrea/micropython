# Unexpected exception with 'in' operator on float
# VM was too eager to optimise float binary ops

print(0.0 in ())
print(0.0 in [])
print(0.0 in {})
print(0.0 in range(4))
print(0.0 not in ())
print(0.0 not in [])
print(0.0 not in {})
print(0.0 not in range(4))
