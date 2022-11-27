#!/bin/sh
#
# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.

# Parameters for performance tests:
# - param_n is the approximate frequency of the target CPU in MHz, and sets
#   how many loops the test will do.
# - param_n is the approximate amount of heap the target has, in KiB, and
#   sets how much memory the test will use.
# Values of N=100 and M=100 are the standard choice of parameters and should
# be appropriate for most LEON targets.  They can be changed if needed.
param_n=100
param_m=100

######## Parse arguments.

verbose=1
run_mode="local"

while [[ $# > 0 ]]; do
    arg="$1"
    case $arg in
        -q)
        verbose=0
        ;;
        -r|--remote)
        run_mode="remote"
        ;;
        *)
        echo "unknown option: $arg"
        exit 1
        ;;
    esac
    shift
done

######## Run or prepare tests.

# Performance tests to run.
tests="
    bm_chaos.py
    bm_fannkuch.py
    bm_fft.py
    bm_float.py
    bm_hexiom.py
    bm_nqueens.py
    bm_pidigits.py
    misc_aes.py
    misc_mandel.py
    misc_pystone.py
    misc_raytrace.py
    "

all_py_files=""

if [ $run_mode = local ]; then
    # Header needed to make output machine readable.
    echo "N=$param_n M=$param_m n_average=1"
fi

# Go through each test and either run it or prepare it for remote execution.
for testfile in $tests; do
    basename=`basename $testfile .py`
    temp=`mktemp temp_perf_${basename}_XXXX.py`
    cat CORE_PERFBENCH/$testfile CORE_PERFBENCH/benchrun.py > $temp
    echo "bm_run($param_n, $param_m)" >> $temp
    if [ $run_mode = local ]; then
        # Run performance benchmark test locally, one at a time.
        echo -n "$testfile: "
        ./run-tests.sh --no-test-name --show-output $temp
        /bin/rm $temp
    else
        # Combine .py files to execute remotely all together.
        all_py_files="$all_py_files $temp"
    fi
done

if [ $run_mode = remote ]; then
    # Prepare performance benchmark tests for remote execution.
    q=""
    if [ $verbose = 0 ]; then
        q="-q"
    fi
    ./remote-tests-prepare.sh $q --no-exp -o leon_tests_${MICROPY_RTEMS_VER}_perfbench $all_py_files
    /bin/rm $all_py_files
fi
