# basic dictionary

d = {}
print(d)
d[2] = 123
print(d)
d = {1:2}
d[3] = 3
print(len(d), d[1], d[3])
d[1] = 0
print(len(d), d[1], d[3])

x = 1
while x < 1000:
    d[x] = x
    x += 1
print(d[500])
