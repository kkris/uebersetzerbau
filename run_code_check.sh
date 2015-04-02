#!/bin/bash

TESTS=${1%/} # remove trailing /
BIN=$2

for expected in $TESTS/*.code
do
    testcase="${expected%.code}.0"

    $BIN < $testcase 2> /dev/null | python ../testcode.py --check-code $expected
done

exit 0


