#!/usr/bin/env python2
#
# mpy_package.py: tool to package MicroPython .mpy files for use on a LEON target

from __future__ import print_function
import sys
import struct

MPY_MEM_PACKED_HEADER = 0x5041434b  # PACK in ASCII


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


def make_srec(addr, data):
    for i in range(0, len(data), 16):
        print(make_srec_line(3, addr + i, data[i : i + 16]))
    # emit S7 with 0 as "execution addr" to indicate end of S3 records
    #print(make_srec_line(7, 0, ''))


def convert_mpy_to_srec(addr, filename):
    with open(filename, 'rb') as f:
        data = f.read()
    data = struct.pack('>L', len(data)) + data
    make_srec(addr, data)


def convert_mpy_to_packed(num_tasks, args):
    prefix_data = []
    mpy_bytes = b''

    # header to indicate packed
    prefix_data.append(struct.pack('>LLL', MPY_MEM_PACKED_HEADER, num_tasks, len(args)))

    # create index table (pair of offset,len for each file)
    word_size = 4
    mpy_data_start = (3 + len(args) * 2) * word_size
    for filename in args:
        with open(filename, 'rb') as f:
            data = f.read()
        prefix_data.append(struct.pack('>LL', mpy_data_start + len(mpy_bytes), len(data)))
        mpy_bytes += data

    return b''.join(prefix_data) + mpy_bytes


def usage():
    print('''\
usage: mpy_package.py <command> <args ...>
commands:
    mpy_package.py tohdr <mpy file> ...
    mpy_package.py tosrec <start_addr> <addr_increment> <mpy file> ...
    mpy_package.py tobin <start_addr> <addr_increment> <mpy file> ...
    mpy_package.py tosrecpacked <start_addr> <num_tasks> <mpy file> ...
    mpy_package.py tobinpacked <start_addr> <num_tasks> <mpy file> ...''', file=sys.stderr)
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


def main_tosrecpacked(addr, num_tasks, args):
    data = convert_mpy_to_packed(num_tasks, args)
    make_srec(addr, data)


def main_tobinpacked(addr, num_tasks, args):
    # addr is not needed
    sys.stdout.write(convert_mpy_to_packed(num_tasks, args))


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
    elif sys.argv[1] in ('tosrecpacked', 'tobinpacked'):
        try:
            addr = int(sys.argv[2], 0)
            num_tasks = int(sys.argv[3], 0)
        except ValueError:
            usage()
        if sys.argv[1] == 'tosrecpacked':
            main_tosrecpacked(addr, num_tasks, sys.argv[4:])
        else:
            main_tobinpacked(addr, num_tasks, sys.argv[4:])
    else:
        usage()


if __name__ == '__main__':
    main()
