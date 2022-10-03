#!/bin/sh

# Core MicroPython tests.
./run-tests.sh CORE_BASICS/*.py
./run-tests.sh CORE_UNICODE/*.py
./run-tests.sh CORE_FLOAT/*.py
./run-tests.sh CORE_EXTMOD/*.py
./run-tests.sh CORE_MISC/*.py
./run-tests.sh CORE_MICROPYTHON/*.py
./run-tests.sh CORE_STRESS/*.py
./run-tests.sh cpydiff/*.py

# micropython-ulab tests.
./run-tests.sh `find ../lib/micropython-ulab/tests -name \*.py | sort`

# MicroPython performance tests (they have no expected output).
./run-perfbench.sh

# LEON and RTEMS specific tests.
./run-tests.sh spacebel_tickets/*.py
./run-tests.sh leon_t1/*.exp
./run-tests.sh -t 2 leon_t2/*.exp
./run-tests.sh -t 10 leon_t10/*.exp

# LEON performance tests (they have no expected output).
./run-tests.sh -o leon_perf/*.py

# mpy-cross tests.
./run-tests-mpy-cross.sh mpy_cross/*.py
