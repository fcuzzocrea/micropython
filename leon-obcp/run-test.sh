#!/bin/sh

RM=/bin/rm
outfile=obcp_01_test.out
expfile=obcp_01_test.exp

echo "Running test: OBCP_01"

make run | grep -a "^UART A sent : " > $outfile

diff --brief $outfile $expfile > /dev/null

if [ $? -eq 0 ]; then
    echo "test passed"
    $RM $outfile
else
    echo "test failed"
fi
