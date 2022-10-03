#!/bin/sh
#
# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.

######## settings

RM="rm -f"
MPC="../mpy-cross/mpy-cross"
MPY_PACKAGE="../leon-common/mpy_package.py"
UNHEXLIFY="./unhexlify.py"

######## parse arguments

num_tasks=1
output_processing="diff"
output_test_name="yes"
tests=""

while [[ $# > 0 ]]; do
    arg="$1"
    case $arg in
        -t|--num-tasks)
        num_tasks="$2"
        shift
        ;;
        -r|--raw-output)
        output_processing="raw"
        ;;
        -o|--show-output)
        output_processing="show"
        ;;
        --no-test-name)
        output_test_name="no"
        ;;
        -*)
        echo "unknown option: $arg"
        exit 1
        ;;
        *)
        tests="$tests $arg"
        ;;
    esac
    shift
done

if [ -z "$MICROPY_RTEMS_VER" ]; then
    echo "MICROPY_RTEMS_VER not set"
    exit 1
fi

if [ -z "$tests" ]; then
    echo "no tests specified"
    exit 1
fi

######## Set up toolchain based on MICROPY_RTEMS_VER

# Default script load addresses.
ADDR_TASK1="0x40200000"
ADDR_INCREMENT="0x10000"

if [ $MICROPY_RTEMS_VER = RTEMS_4_8_EDISOFT ]; then
    TARGET=leon2
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-4.8
elif [ $MICROPY_RTEMS_VER = RTEMS_4_8 ]; then
    TARGET=leon2
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-4.8
elif [ $MICROPY_RTEMS_VER = RTEMS_4_10 ]; then
    TARGET=leon2
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-4.10
elif [ $MICROPY_RTEMS_VER = RTEMS_4_11 ]; then
    TARGET=leon2
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-4.11
elif [ $MICROPY_RTEMS_VER = RTEMS_5_1_GR712RC ]; then
    TARGET=sis-leon3
    MICROPY_RTEMS_ROOT_DEFAULT=/opt/rtems-5.1-2019.07.25
    RTEMS_API=5
elif [ $MICROPY_RTEMS_VER = RTEMS_6_GR712RC ]; then
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

# Configure $SIS and $OBJCOPY executables if needed.
if [ -n "$RTEMS_API" ]; then
    SIS=$MICROPY_RTEMS_ROOT/bin/sparc-rtems${RTEMS_API}-sis
    OBJCOPY=$MICROPY_RTEMS_ROOT/bin/sparc-rtems${RTEMS_API}-objcopy
fi

if [ -z "$MICROPY_LAYSIM" ]; then
    MICROPY_LAYSIM=/opt/laysim-gr740/laysim-gr740-dbt-cli
fi

######## run tests

build_dir=../leon-for-tests/build-$MICROPY_RTEMS_VER

function prepare_scripts {
    local basename=$1
    shift
    if [ $TARGET = leon2 ]; then
        # leon2-emu requires an srec input file, so create one.
        $MPY_PACKAGE tosrec $ADDR_TASK1 $ADDR_INCREMENT $@ > $basename.srec || exit $?
    else
        # sis requires a single elf input file, so add compiled scripts to firmware.elf.
        $MPY_PACKAGE tobin $ADDR_TASK1 $ADDR_INCREMENT $@ > $basename.bin || exit $?
        $OBJCOPY \
            --add-section .scripts=$basename.bin \
            --change-section-address .scripts=$ADDR_TASK1 \
            --set-section-flags .scripts=contents,alloc,load,data \
            $build_dir/firmware.elf $basename.elf 2> /dev/null
        $RM $basename.bin
    fi
}

function run_leon {
    if [ $TARGET = leon2 ]; then
        leon2_emu_cmd=$(cat <<-EOF
            load /srec "$build_dir/firmware.srec"
            load /symtab "$build_dir/firmware.tab"
            load /srec "$1.srec"
            bre %code(leon_emu_terminate) /TAG=_emu_terminate /CMD={ bre /exit/stop }
            set pc=0x40000000
            set i6=0x41000000
            set o6=0x40FFFE80
            step 500000000
EOF
        )
        echo -e "$leon2_emu_cmd" | leon2-emu
    elif [ $TARGET = sis-leon3 ]; then
        $SIS -leon3 -dumbio -r $1.elf
    elif [ $TARGET = sis-gr740 ]; then
        $SIS -gr740 -dumbio -r $1.elf
    elif [ $TARGET = laysim-gr740 ]; then
        $MICROPY_LAYSIM -r -core0 $1.elf
    else
        echo "Unknown target: $TARGET"
    fi
}

numtests=0
numtestcases=0
numpassed=0
numskipped=0
numfailed=0
nameskipped=
namefailed=

for testfile in $tests
do
    # strip of .exp or .py to allow either to be passed in
    basename=$(basename $testfile .exp)
    basename=$(basename $basename .py)

    infile_no_ext=$(dirname $testfile)/$basename
    outfile=${basename}_$$.out
    tempfile=temp_$$_${basename}

    if [ $num_tasks = 1 ]; then
        $MPC ${infile_no_ext}.py || exit $?
        prepare_scripts ${tempfile} \
            ${infile_no_ext}.mpy
    elif [ $num_tasks = 2 ]; then
        $MPC ${infile_no_ext}.1.py || exit $?
        $MPC ${infile_no_ext}.2.py || exit $?
        prepare_scripts ${tempfile} \
            ${infile_no_ext}.1.mpy \
            ${infile_no_ext}.2.mpy
    elif [ $num_tasks = 10 ]; then
        $MPC ${infile_no_ext}.py || exit $?
        prepare_scripts ${tempfile} \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy \
            ${infile_no_ext}.mpy
    else
        echo "bad num_tasks"
        exit 1
    fi

    if [ $output_processing != "diff" -a $output_test_name = "yes" ]; then
        # Print test name.
        echo "$infile_no_ext:"
    fi

    if [ $output_processing = "raw" ]; then
        # Let test output go directly to stdout.
        run_leon ${tempfile}
    elif [ $output_processing = "show" ]; then
        # Pipe output through unhexlify, then to stdout.
        run_leon ${tempfile} | $UNHEXLIFY $TARGET
    else
        # Redirect output to a file for diff'ing.
        run_leon ${tempfile} | $UNHEXLIFY $TARGET > $outfile
    fi

    # clean up temp script containing mpy file
    $RM $infile_no_ext.mpy $tempfile.srec $tempfile.elf

    if [ $output_processing != "diff" ]; then
        continue
    fi

    # Find the file with the expected test output.
    if [ -r ${infile_no_ext}.exp ]; then
        expfile=${infile_no_ext}.exp
    elif [ -r ${infile_no_ext}.py.exp ]; then
        expfile=${infile_no_ext}.py.exp
    else
        echo "Missing .exp file for test ${infile_no_ext}"
        exit 1
    fi

    numtestcases=$(expr $numtestcases + $(cat $expfile | wc -l))

    diff --brief $expfile $outfile > /dev/null

    if [ $? -eq 0 ]
    then
        echo "pass  $infile_no_ext"
        $RM $outfile
        numpassed=$(expr $numpassed + 1)
    else
        echo "FAIL  $infile_no_ext"
        numfailed=$(expr $numfailed + 1)
        namefailed="$namefailed $basename"
    fi

    numtests=$(expr $numtests + 1)
done

if [ $output_processing != "diff" ]; then
    exit 0
fi

echo "$numtests tests performed ($numtestcases individual testcases)"
echo "$numpassed tests passed"
if [ $numskipped != 0 ]
then
    echo "$numskipped tests skipped -$nameskipped"
fi
if [ $numfailed != 0 ]
then
    echo "$numfailed tests failed -$namefailed"
    exit 1
else
    exit 0
fi
