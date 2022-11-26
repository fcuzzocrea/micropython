#!/bin/sh
#
# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
#
# This script combines a set of Python test scripts with a test runner
# executable to create a stand-alone application that runs all the tests.
# It also creates the corresponding expected output for the test run.

######## Settings.

RM="rm -f"
MPY_CROSS="../mpy-cross/mpy-cross"
MPY_PACKAGE="../leon-common/mpy_package.py"

######## Parse arguments.

verbose=1
num_tasks=1
output_basename="leon_test_firmware"
test_list=""
process_exp=1

while [[ $# > 0 ]]; do
    arg="$1"
    case $arg in
        -q)
        verbose=0
        ;;
        -t|--num-tasks)
        num_tasks="$2"
        shift
        ;;
        -o)
        output_basename="$2"
        shift
        ;;
        --no-exp)
        process_exp=0
        ;;
        -*)
        echo "unknown option: $arg"
        exit 1
        ;;
        *)
        test_list="$test_list $arg"
        ;;
    esac
    shift
done

if [ -z "$MICROPY_RTEMS_VER" ]; then
    echo "MICROPY_RTEMS_VER not set"
    exit 1
fi

if [ -z "$test_list" ]; then
    echo "no tests specified"
    exit 1
fi

######## Set up toolchain based on MICROPY_RTEMS_VER

# Default script load addresses.
ADDR_TASK1="0x40200000"

if [ $MICROPY_RTEMS_VER = RTEMS_6_GR712RC ]; then
    TARGET=sis-leon3
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-6-sparc-gr712rc-smp-3
    RTEMS_API=6
elif [ $MICROPY_RTEMS_VER = RTEMS_6_GR740 ]; then
    TARGET=sis-gr740
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-6-sparc-gr740-smp-3
    RTEMS_API=6
    ADDR_TASK1="0x00200000"
else
    echo "Unknown MICROPY_RTEMS_VER: $MICROPY_RTEMS_VER"
    exit 1
fi

# Use default RTEMS root directory if nothing is specified in $MICROPY_RTEMS_ROOT.
if [ -z "$MICROPY_RTEMS_ROOT" ]; then
    MICROPY_RTEMS_ROOT=$MICROPY_RTEMS_ROOT_DEFAULT
fi

# Configure $OBJCOPY executable.
OBJCOPY=$MICROPY_RTEMS_ROOT/bin/sparc-rtems${RTEMS_API}-objcopy

# Configure location of main test firmware elf file.
TEST_FIRMWARE_ELF=../leon-for-tests/build-$MICROPY_RTEMS_VER/firmware.elf

######## Compile all input .py files to .mpy files.

temp_dir=`mktemp -d temp_leon_tests_XXXXXX`
all_mpy_files=
all_mpy_bin=$temp_dir/all_mpy.bin
output_elf=$output_basename.elf
output_exp=$output_basename.exp

echo -n "" > $output_exp

# Sort tests so they run in a deterministic order.
test_list=`echo $test_list | tr ' ' '\n' | LC_ALL=C sort`

((test_index=0))
for testfile in $test_list
do
    # Strip off .exp or .py to allow either to be passed in.
    basename=$(basename $testfile .exp)
    basename=$(basename $basename .py)
    infile_no_ext=$(dirname $testfile)/$basename

    if [ $verbose = 1 ]; then
        echo "Preparing $infile_no_ext"
    fi

    # Compile .py to .mpy, and add .mpy to list of .mpy files.
    outfile=`mktemp -p $temp_dir test_XXXXXX`
    if [ $num_tasks = 1 ]; then
        # The cases t=1 has a single .py file.
        $MPY_CROSS -o $outfile.mpy $infile_no_ext.py || exit $?
        all_mpy_files="$all_mpy_files $outfile.mpy"
    elif [ $num_tasks = 2 ]; then
        # The case t=2 has two .py files, one for each task.
        $MPY_CROSS -o $outfile.1.mpy $infile_no_ext.1.py || exit $?
        $MPY_CROSS -o $outfile.2.mpy $infile_no_ext.2.py || exit $?
        all_mpy_files="$all_mpy_files $outfile.1.mpy $outfile.2.mpy"
    elif [ $num_tasks = 10 ]; then
        # The case t=10 has a single .py file, while is run on all 10 tasks.
        $MPY_CROSS -o $outfile.mpy $infile_no_ext.py || exit $?
        for i in {1..10}; do
            all_mpy_files="$all_mpy_files $outfile.mpy"
        done
    else
        echo "bad num_tasks"
        exit 1
    fi

    # Print test index and name.
    echo -n "======== Expected output for " >> $output_exp
    if [ $num_tasks == 1 ]; then
        echo -n "script $test_index" >> $output_exp
    else
        echo -n "scripts $test_index-`expr $test_index + $num_tasks - 1`" >> $output_exp
    fi
    echo -n " : $infile_no_ext : " >> $output_exp

    if [ $process_exp = 1 ]; then
        # Find the file with the expected test output.
        if [ -r $infile_no_ext.exp ]; then
            expfile=$infile_no_ext.exp
        elif [ -r $infile_no_ext.py.exp ]; then
            expfile=$infile_no_ext.py.exp
        else
            echo "Missing .exp file for test $infile_no_ext"
            exit 1
        fi

        # Add .exp file to combined .exp file.
        echo "`cat $expfile | wc -l` lines" >> $output_exp
        cat $expfile >> $output_exp
    else
        echo "0 lines" >> $output_exp
    fi

    ((test_index+=$num_tasks))
done

######## Combine all .mpy files with input firmware to produce output firmware.

$MPY_PACKAGE tobinpacked $ADDR_TASK1 $num_tasks $all_mpy_files > $all_mpy_bin || exit $?
$OBJCOPY \
    --add-section .scripts=$all_mpy_bin \
    --change-section-address .scripts=$ADDR_TASK1 \
    --set-section-flags .scripts=contents,alloc,load,data \
    $TEST_FIRMWARE_ELF $output_elf 2> /dev/null

######## Clean up and print a usage message.

$RM -r $temp_dir

if [ $verbose = 1 ]; then
    echo ""
    echo "Application and tests have been bundled and put in: $output_elf"
    echo "Expected output of this executable is in:           $output_exp"
    echo ""
    echo "The executable is stand-alone and can be executed."
    echo "It can be run on a simulator, eg laysim-gr740-dbt-cli -r -core0 $output_elf"
    echo "Or it can be run directly on hardware."
else
    echo $output_elf
fi
