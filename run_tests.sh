#!/bin/bash

TESTS=${1%/} # remove trailing /
BIN=$2

for t in $TESTS/*
do
    testcase="${t##*/}"
    expected="${t: -1}"

    $BIN < $t 2&> /dev/null

    actual="$?"

    if [ "$actual" == "$expected" ]
    then
        echo "OK [$actual] $testcase"
    else
        echo "FF [$actual] $testcase"
    fi

done

exit 0
