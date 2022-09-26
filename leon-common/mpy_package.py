#!/usr/bin/env python2
#
# mpy_package.py: tool to package MicroPython .mpy files for use on a LEON target

from __future__ import print_function
import sys
import struct

def convert_mpy_to_c_header(filenames):
    scripts = []
    for filename in filenames:
        with open(filename, 'rb') as f:
            data = f.read()
        scripts.append(data)

    print('static const size_t mpy_script_num = %u;' % len(scripts))
    print('static const size_t mpy_script_len[] = {',
        ', '.join(str(len(script)) for script in scripts), '};')
    print('static const uint8_t *mpy_script_data[] = {')
    for script in scripts:
        out = '(const uint8_t*)"'
        for b in script:
            out += '\\x%02x' % ord(b)
        out += '",'
        print('   ', out)
    print('\n};')


def make_srec_line(type, addr, data):
    data = struct.pack('>BL', 4 + len(data) + 1, addr) + data
    str = 'S%u' % type
    checksum = 0
    for b in data:
        val = ord(b)
        str += '%02X' % val
        checksum = (checksum + val) & 0xff
    checksum = (~checksum) & 0xff
    str += '%02X' % (checksum,)
    return str


def convert_mpy_to_srec(addr, filename):
    with open(filename, 'rb') as f:
        data = f.read()
    data = struct.pack('>L', len(data)) + data
    for i in range(0, len(data), 16):
        print(make_srec_line(3, addr + i, data[i : i + 16]))
    # emit S7 with 0 as "execution addr" to indicate end of S3 records
    #print(make_srec_line(7, 0, ''))


def usage():
    print('''\
usage: mpy_package.py <command> <args ...>
commands:
    mpy_package.py tohdr <mpy file> ...
    mpy_package.py tosrec <start_addr> <addr_increment> <mpy file> ...
    mpy_package.py tobin <start_addr> <addr_increment> <mpy file> ...''', file=sys.stderr)
    sys.exit(1)


def main_tohdr(args):
    convert_mpy_to_c_header(args)


def main_tosrec(addr, addr_increment, args):
    for filename in args:
        convert_mpy_to_srec(addr, filename)
        addr += addr_increment


def main_tobin(addr, addr_increment, args):
    last_addr = None
    all_data = []
    for filename in args:
        with open(filename, 'rb') as f:
            data = f.read()
        if last_addr is not None:
            # pad
            all_data.append(b'\x00' * (addr - last_addr))
        all_data.append(struct.pack('>L', len(data)) + data)
        last_addr = addr + len(all_data[-1])
        addr += addr_increment
    sys.stdout.write(b''.join(all_data))


def main():
    if len(sys.argv) < 2:
        usage()
    if sys.argv[1] == 'tohdr':
        main_tohdr(sys.argv[2:])
    elif sys.argv[1] in ('tosrec', 'tobin'):
        try:
            addr = int(sys.argv[2], 0)
            addr_increment = int(sys.argv[3], 0)
        except ValueError:
            usage()
        if sys.argv[1] == 'tosrec':
            main_tosrec(addr, addr_increment, sys.argv[4:])
        else:
            main_tobin(addr, addr_increment, sys.argv[4:])
    else:
        usage()


if __name__ == '__main__':
    main()
