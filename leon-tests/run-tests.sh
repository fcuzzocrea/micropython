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

ADDR_TASK1="0x40200000"
ADDR_TASK2="0x40210000"
ADDR_TASK3="0x40220000"
ADDR_TASK4="0x40230000"
ADDR_TASK5="0x40240000"
ADDR_TASK6="0x40250000"
ADDR_TASK7="0x40260000"
ADDR_TASK8="0x40270000"
ADDR_TASK9="0x40280000"
ADDR_TASK10="0x40290000"

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

######## run tests

build_dir=../leon-for-tests/build-$MICROPY_RTEMS_VER

leon2_emu_cmd=$(cat <<-EOF
    load /srec "$build_dir/firmware.srec"
    load /symtab "$build_dir/firmware.tab"
    load /srec "script.srec"
    bre %code(leon_emu_terminate) /TAG=_emu_terminate /CMD={ bre /exit/stop }
    set pc=0x40000000
    set i6=0x41000000
    set o6=0x40FFFE80
    step 500000000
EOF
)

function run_leon {
    echo -e "$leon2_emu_cmd" | leon2-emu
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
    expfile=${infile_no_ext}.exp
    outfile=${basename}.out

    if [ $num_tasks = 1 ]; then
        $MPC ${infile_no_ext}.py || exit $?
        $MPY_PACKAGE tosrec $ADDR_TASK1 ${infile_no_ext}.mpy > script.srec || exit $?
    elif [ $num_tasks = 2 ]; then
        $MPC ${infile_no_ext}.1.py || exit $?
        $MPC ${infile_no_ext}.2.py || exit $?
        $MPY_PACKAGE tosrec $ADDR_TASK1 ${infile_no_ext}.1.mpy \
            $ADDR_TASK2 ${infile_no_ext}.2.mpy > script.srec || exit $?
    elif [ $num_tasks = 10 ]; then
        $MPC ${infile_no_ext}.py || exit $?
        $MPY_PACKAGE tosrec \
            $ADDR_TASK1 ${infile_no_ext}.mpy \
            $ADDR_TASK2 ${infile_no_ext}.mpy \
            $ADDR_TASK3 ${infile_no_ext}.mpy \
            $ADDR_TASK4 ${infile_no_ext}.mpy \
            $ADDR_TASK5 ${infile_no_ext}.mpy \
            $ADDR_TASK6 ${infile_no_ext}.mpy \
            $ADDR_TASK7 ${infile_no_ext}.mpy \
            $ADDR_TASK8 ${infile_no_ext}.mpy \
            $ADDR_TASK9 ${infile_no_ext}.mpy \
            $ADDR_TASK10 ${infile_no_ext}.mpy > script.srec || exit $?
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
        run_leon
    elif [ $output_processing = "show" ]; then
        # Pipe output through unhexlify, then to stdout.
        run_leon | $UNHEXLIFY
    else
        # Redirect output to a file for diff'ing.
        run_leon | $UNHEXLIFY > $outfile
    fi

    # clean up temp script containing mpy file
    $RM script.srec

    if [ $output_processing != "diff" ]; then
        continue
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
