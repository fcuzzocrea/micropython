#!/usr/bin/env python2

import sys
import binascii


PREFIXES = {
    "leon2": "UART A sent : HEX ",
    "sis-leon3": "HEX ",
    "sis-gr740": "HEX ",
}

target = sys.argv[1]
prefix = PREFIXES[target]

lines = []
for line in sys.stdin:
    if line.startswith(prefix):
        lines.append(line[len(prefix) :].rstrip())
sys.stdout.write(binascii.unhexlify("".join(lines)))
