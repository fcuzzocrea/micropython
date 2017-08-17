# tuples didn't work as arguments of namedtuple

import ucollections
N = ucollections.namedtuple('N', ('a', 'b', 'c'))
print(N(1, 2, 3))
