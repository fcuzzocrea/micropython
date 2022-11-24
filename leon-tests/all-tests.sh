#!/bin/sh

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
./run-tests.sh $ulab_tests

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
