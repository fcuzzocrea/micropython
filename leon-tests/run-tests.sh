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
MPYTOOL="../tools/mpytool.py"
UNHEXLIFY="../tools/unhexlify.py"

ADDR_TASK1="0x40100000"
ADDR_TASK2="0x40110000"
ADDR_TASK3="0x40120000"
ADDR_TASK4="0x40130000"
ADDR_TASK5="0x40140000"
ADDR_TASK6="0x40150000"
ADDR_TASK7="0x40160000"
ADDR_TASK8="0x40170000"
ADDR_TASK9="0x40180000"
ADDR_TASK10="0x40190000"

######## parse arguments

num_tasks=1
output_processing=0
tests=""

while [[ $# > 0 ]]; do
    arg="$1"
    case $arg in
        -t|--num-tasks)
        num_tasks="$2"
        shift
        ;;
        -r|--raw-output)
        output_processing=1
        ;;
        -o|--show-output)
        output_processing=2
        ;;
        *)
        tests="$tests $arg"
        ;;
    esac
    shift
done

if [ -z "$tests" ]; then
    echo "no tests specified"
    exit 1
fi

######## run tests

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
        $MPYTOOL tosrec $ADDR_TASK1 ${infile_no_ext}.mpy > script.srec || exit $?
    elif [ $num_tasks = 2 ]; then
        $MPC ${infile_no_ext}.1.py || exit $?
        $MPC ${infile_no_ext}.2.py || exit $?
        $MPYTOOL tosrec $ADDR_TASK1 ${infile_no_ext}.1.mpy \
            $ADDR_TASK2 ${infile_no_ext}.2.mpy > script.srec || exit $?
    elif [ $num_tasks = 10 ]; then
        $MPC ${infile_no_ext}.py || exit $?
        $MPYTOOL tosrec \
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

    if [ $output_processing = 1 ]; then
        echo "$infile_no_ext:"
        cat cmd.txt | leon2-emu
        continue
    elif [ $output_processing = 2 ]; then
        echo "$infile_no_ext:"
        cat cmd.txt | leon2-emu | $UNHEXLIFY
        continue
    else
        cat cmd.txt | leon2-emu | $UNHEXLIFY > $outfile
    fi

    # clean up temp script containing mpy file
    $RM script.srec

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

if [ $output_processing != 0 ]; then
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
