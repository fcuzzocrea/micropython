# Test for MicroPython port to LEON
# Test ID: NOHEAP_06

e = None

def foo():
    try:
        a
    except NameError as e:
        print('NameError:', e)

import micropython
micropython.heap_lock()

try:
    a
except:
    print('Exception')

try:
    a
except NameError:
    print('NameError')

try:
    a
except NameError as e:
    print('NameError:', e)

foo()
