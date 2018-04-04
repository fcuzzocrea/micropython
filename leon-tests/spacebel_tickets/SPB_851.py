print({1:2, 3:4}.keys() == {1:20, 3:40}.keys())
print({1:2, 3:4}.keys() != {1:20, 3:40}.keys())
print({1:2, 3:4}.items() == {1:2, 3:4}.items())
print({1:2, 3:4}.items() != {1:2, 3:4}.items())

# these should evaluate to False
print({1:2}.keys() == {1:2, 3:4}.values())
print({1:2}.keys() == {1:2, 3:4}.keys())
print({1:2}.keys() == {2:3}.keys())
