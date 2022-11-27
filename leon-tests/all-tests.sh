#!/bin/sh
#
# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
#
# This script runs all tests, either locally on a simulator, or prepares them
# for remote execution.

######## Parse arguments.

run_mode="local"

while [[ $# > 0 ]]; do
    arg="$1"
    case $arg in
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

######## Run tests.

# Define the spacebel tests to run.
spacebel_tests="
    spacebel_tickets/SPB_1000.py
    spacebel_tickets/SPB_1001.py
    spacebel_tickets/SPB_1002.py
    spacebel_tickets/SPB_1458.py
    spacebel_tickets/SPB_1550.py
    spacebel_tickets/SPB_1593.py
    spacebel_tickets/SPB_1603.py
    spacebel_tickets/SPB_1604.py
    spacebel_tickets/SPB_1651.py
    spacebel_tickets/SPB_1689.py
    spacebel_tickets/SPB_1852.py
    spacebel_tickets/SPB_2000.py
    spacebel_tickets/SPB_542.py
    spacebel_tickets/SPB_543.py
    spacebel_tickets/SPB_544.py
    spacebel_tickets/SPB_553.py
    spacebel_tickets/SPB_561.py
    spacebel_tickets/SPB_562.py
    spacebel_tickets/SPB_563.py
    spacebel_tickets/SPB_566.py
    spacebel_tickets/SPB_567.py
    spacebel_tickets/SPB_572.py
    spacebel_tickets/SPB_702.py
    spacebel_tickets/SPB_709.py
    spacebel_tickets/SPB_711.py
    spacebel_tickets/SPB_713.py
    spacebel_tickets/SPB_714.py
    spacebel_tickets/SPB_716.py
    spacebel_tickets/SPB_717.py
    spacebel_tickets/SPB_721.py
    spacebel_tickets/SPB_851.py
    spacebel_tickets/SPB_852.py
    spacebel_tickets/SPB_854.py
    spacebel_tickets/SPB_861.py
    spacebel_tickets/SPB_862.py
    spacebel_tickets/SPB_868.py
    spacebel_tickets/SPB_869.py
    spacebel_tickets/SPB_871.py
    spacebel_tickets/SPB_872.py
    spacebel_tickets/SPB_877.py
    spacebel_tickets/SPB_883.py
    spacebel_tickets/SPB_942.py
    spacebel_tickets/SPB_946.py
    spacebel_tickets/SPB_963.py
    spacebel_tickets/SPB_996.py
    spacebel_tickets/SPB_997.py
    spacebel_tickets/SPB_998.py
    spacebel_tickets/SPB_999.py
    spacebel_tickets/SPB_extra.py
    "

# Add spacebel tests that pass only locally.
if [ $run_mode = local ]; then
    spacebel_tests="$spacebel_tests spacebel_tickets/SPB_1781.py"
    spacebel_tests="$spacebel_tests spacebel_tickets/SPB_712.py"
fi

# Define the ulab tests to run.  They should all pass.
ulab_test_dir=../lib/micropython-ulab/tests
ulab_tests="
    $ulab_test_dir/1d/complex/complex_exp.py
    $ulab_test_dir/1d/complex/complex_sqrt.py
    $ulab_test_dir/1d/complex/imag_real.py
    $ulab_test_dir/1d/numpy/00smoke.py
    $ulab_test_dir/1d/numpy/argminmax.py
    $ulab_test_dir/1d/numpy/compare.py
    $ulab_test_dir/1d/numpy/convolve.py
    $ulab_test_dir/1d/numpy/fft.py
    $ulab_test_dir/1d/numpy/gc.py
    $ulab_test_dir/1d/numpy/interp.py
    $ulab_test_dir/1d/numpy/optimize.py
    $ulab_test_dir/1d/numpy/poly.py
    $ulab_test_dir/1d/numpy/slicing.py
    $ulab_test_dir/1d/numpy/slicing2.py
    $ulab_test_dir/1d/numpy/sum.py
    $ulab_test_dir/1d/numpy/trapz.py
    $ulab_test_dir/1d/numpy/universal_functions.py
    $ulab_test_dir/2d/complex/binary_op.py
    $ulab_test_dir/2d/complex/complex_exp.py
    $ulab_test_dir/2d/complex/complex_sqrt.py
    $ulab_test_dir/2d/complex/conjugate.py
    $ulab_test_dir/2d/complex/imag_real.py
    $ulab_test_dir/2d/complex/sort_complex.py
    $ulab_test_dir/2d/numpy/00smoke.py
    $ulab_test_dir/2d/numpy/any_all.py
    $ulab_test_dir/2d/numpy/arange.py
    $ulab_test_dir/2d/numpy/asarray.py
    $ulab_test_dir/2d/numpy/buffer.py
    $ulab_test_dir/2d/numpy/cholesky.py
    $ulab_test_dir/2d/numpy/concatenate.py
    $ulab_test_dir/2d/numpy/delete.py
    $ulab_test_dir/2d/numpy/diag.py
    $ulab_test_dir/2d/numpy/eye.py
    $ulab_test_dir/2d/numpy/full.py
    $ulab_test_dir/2d/numpy/initialisation.py
    $ulab_test_dir/2d/numpy/isinf.py
    $ulab_test_dir/2d/numpy/linalg.py
    $ulab_test_dir/2d/numpy/linspace.py
    $ulab_test_dir/2d/numpy/logspace.py
    $ulab_test_dir/2d/numpy/methods.py
    $ulab_test_dir/2d/numpy/nonzero.py
    $ulab_test_dir/2d/numpy/numericals.py
    $ulab_test_dir/2d/numpy/ones.py
    $ulab_test_dir/2d/numpy/operators.py
    $ulab_test_dir/2d/numpy/signal.py
    $ulab_test_dir/2d/numpy/size.py
    $ulab_test_dir/2d/numpy/sort.py
    $ulab_test_dir/2d/numpy/where.py
    $ulab_test_dir/2d/numpy/zeros.py
    $ulab_test_dir/2d/scipy/cho_solve.py
    $ulab_test_dir/2d/scipy/solve_triangular.py
    $ulab_test_dir/2d/scipy/sosfilt.py
    $ulab_test_dir/2d/utils/from_buffer.py
    $ulab_test_dir/3d/complex/complex_exp.py
    $ulab_test_dir/3d/complex/complex_sqrt.py
    $ulab_test_dir/3d/complex/imag_real.py
    $ulab_test_dir/3d/numpy/create.py
    $ulab_test_dir/4d/complex/complex_exp.py
    $ulab_test_dir/4d/complex/complex_sqrt.py
    $ulab_test_dir/4d/complex/imag_real.py
    $ulab_test_dir/4d/numpy/create.py
    "

function run_tests {
    if [ $run_mode = local ]; then
        # Run locally with helper script.
        ./run-tests.sh $@
    else
        # Trick to get the last argument and store it in "output".
        for output; do true; done
        # Work out a nice name for the test suite.
        if echo $output | grep -q micropython-ulab; then
            output=ulab
        else
            output=`dirname $output`
        fi
        # Prepare the tests for remote execution.
        remote_output=`./remote-tests-prepare.sh -q -o leon_tests_${MICROPY_RTEMS_VER}_$output $@`
        echo "$remote_output is ready for remote execution"
    fi
}

# Core MicroPython tests.
run_tests CORE_BASICS/*.py
run_tests CORE_UNICODE/*.py
run_tests CORE_FLOAT/*.py
run_tests CORE_EXTMOD/*.py
run_tests CORE_MISC/*.py
run_tests CORE_MICROPYTHON/*.py
run_tests CORE_STRESS/*.py
run_tests cpydiff/*.py

# LEON and RTEMS specific tests.
run_tests $spacebel_tests
run_tests leon_t1/*.exp
run_tests -t 2 leon_t2/*.exp
run_tests -t 10 leon_t10/*.exp

# micropython-ulab tests.
run_tests $ulab_tests

# LEON performance tests (they have no expected output).
if [ $run_mode = local ]; then
    ./run-tests.sh -o leon_perf/*.py
else
    run_tests leon_perf/*.py
fi

# MicroPython performance tests (they have no expected output).
if [ $run_mode = local ]; then
    ./run-perfbench.sh
else
    remote_output=`./run-perfbench.sh --remote -q`
    echo "$remote_output is ready for remote execution"
fi

# mpy-cross tests.
if [ $run_mode = local ]; then
    ./run-tests-mpy-cross.sh mpy_cross/*.py
fi
