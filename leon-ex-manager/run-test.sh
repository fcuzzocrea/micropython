#!/bin/sh

RM=/bin/rm
outfile=vmmanage_test.out
expfile=vmmanage_test.exp

echo "Running test: VM_MANAGER"

make run | grep -a "^UART A sent : " > $outfile

diff --brief $outfile $expfile > /dev/null

if [ $? -eq 0 ]; then
    echo "test passed"
    $RM $outfile
else
    echo "test failed"
fi
