#!/usr/bin/env python2

import sys
import binascii

lines = []
for line in sys.stdin:
    if line.startswith('UART A sent : HEX '):
        lines.append(line[18:-1])
sys.stdout.write(binascii.unhexlify(''.join(lines)))
