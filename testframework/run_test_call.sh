#!/bin/bash

TESTS=${1%/} # remove trailing /
BIN=$2

DIR="$(dirname "$(realpath "$0")")"
TESTDIR=/tmp/uebersetzerbau_tests

mkdir -p $TESTDIR

for testcase in $TESTS/*.call
do
    program="${testcase%.call}.0"
    name="$(basename $testcase)"

    cp "$testcase" $TESTDIR/$name
    sed "s|TESTCASE|$name|" $DIR/testmain.c > $TESTDIR/main.c

    $BIN < $program > $TESTDIR/program.s 2> /dev/null

    gcc -g -o $TESTDIR/testmain $TESTDIR/main.c $TESTDIR/program.s
    $TESTDIR/testmain
done

exit 0


