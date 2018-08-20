# test for miscellaneous nan issues reported in Jul/Aug 2018

nan = float('nan')

# negative number to nan
print((-1.0) ** nan)

# nan containment
print(nan in (nan, 1))
print(nan in [nan, 1])
print(nan in {nan, 1})

# object equality enforces reflexivity
print((nan, 1) == (nan, 1))
print([nan, 1] == [nan, 1])

# count behaves like containment
print([nan, 1].count(nan))
