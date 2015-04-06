#!/bin/bash

TESTS=${1%/} # remove trailing /
BIN=$2

for expected in $TESTS/*.call
do
    testcase="${expected%.call}.0"

    $BIN < $testcase > /tmp/out.s 2> /dev/null

    #gcc -o test raisesig.s test.c /tmp/out.s
    gcc -g -o test test.c /tmp/out.s
    ./test
done

exit 0


