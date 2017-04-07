# test formatting floats with large precision, that it doesn't overflow the buffer

def test(num, num_str):
    if num == float('inf') or num == 0.0 and num_str != '0.0':
        # skip numbers that overflow or underflow the FP precision
        return
    for kind in ('e', 'f', 'g'):
        # check precision either side of the size of the buffer (32 bytes)
        for prec in range(23, 36, 2):
            fmt = '%.' + '%d' % prec + kind
            s = fmt % num
            check = abs(float(s) - num)
            if num > 1:
                check /= num
            if check > 1e-6:
                print('FAIL', num_str, fmt, s, len(s), check)

# check most powers of 10, making sure to include exponents with 3 digits
for e in (-101, -100, -99, -50, -10, -9, -1, 0, 1, 5, 20, 99, 100, 101):
    print(e)
    num = pow(10, e)
    test(num, '1e%d' % e)
