#!/bin/sh
#
# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2018 George Robotics Limited

######## settings

RM="rm -f"
MPC="../mpy-cross/mpy-cross"

######## parse arguments

tests="$@"

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

    $MPC ${infile_no_ext}.py > $outfile 2>&1

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
