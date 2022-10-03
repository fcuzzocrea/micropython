#!/bin/sh
#
# This script will build all components and run all tests.

make -C mpy-cross || exit $?
make -C leon-ex-minimal || exit $?
make -C leon-ex-tasks || exit $?
make -C leon-ex-manager || exit $?
make -C leon-ex-pystone || exit $?
make -C leon-spacebel || exit $?
make -C leon-for-tests || exit $?

(cd leon-tests && ./all-tests.sh)
(cd leon-ex-manager && ./run-test.sh)

# The OBCP example is not supported with Edisoft RTEMS 4.8, or RTEMS 6
if [ "$MICROPY_RTEMS_VER" != "RTEMS_4_8_EDISOFT" \
    -a "$MICROPY_RTEMS_VER" != "RTEMS_6_GR712RC" \
    -a "$MICROPY_RTEMS_VER" != "RTEMS_6_GR740" ]; then
    make -C leon-obcp || exit $?
    (cd leon-obcp && ./run-test.sh)
fi
