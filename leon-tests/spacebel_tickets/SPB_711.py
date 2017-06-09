# Unexpected exception when putting float in a set or dictionary
# floats were not hashable

print({0.0})
print({0.0:None})
print(hash(0.0))
