#!/bin/sh

RM=/bin/rm
outfile=vmmanage_test.out
expfile=vmmanage_test.exp

echo "Running test: VM_MANAGER"

make run | awk '/MicroPython manager task started/,/MicroPython manager task ended/{gsub(/\r/,""); print $0}' | sed 's/UART A sent : //' > $outfile

diff --brief $outfile $expfile > /dev/null

if [ $? -eq 0 ]; then
    echo "test passed"
    $RM $outfile
else
    echo "test failed"
fi
