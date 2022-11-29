#!/usr/bin/env python2

import sys
import binascii


PREFIXES = {
    "leon2": "UART A sent : HEX ",
    "sis-leon3": "HEX ",
    "sis-gr740": "HEX ",
    "laysim-gr740": "HEX ",
}

target = "laysim-gr740"

if len(sys.argv) > 1:
    target = sys.argv[1]

prefix = PREFIXES[target]

lines = []
for line in sys.stdin:
    if line.startswith(prefix):
        lines.append(line[len(prefix) :].rstrip())

# Check the last line for any error messages (non-valid hexlify output).
error = None
if lines:
    last_line = lines[-1]
    last_valid_index = 0
    for i in range(0, len(last_line), 2):
        try:
            binascii.unhexlify(last_line[i:i+2])
            last_valid_index = i + 2
        except TypeError:
            break
    if last_valid_index < len(last_line):
        # Last line has an error message, so extract it.
        lines[-1] = last_line[: last_valid_index]
        error = "\nError: " + last_line[last_valid_index:]

sys.stdout.write(binascii.unhexlify("".join(lines)))
if error:
    print(error)
