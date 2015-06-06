#!/bin/bash

TESTCASE=$1
BIN=$2

DIR="$(dirname "$(realpath "$0")")"
TESTDIR=/tmp/uebersetzerbau_tests

mkdir -p $TESTDIR

program="${TESTCASE%.call}.0"
name="$(basename $TESTCASE)"

cp "$TESTCASE" $TESTDIR/$name
sed "s|TESTCASE|$name|" $DIR/testmain.c > $TESTDIR/main.c

$BIN < $program > $TESTDIR/program.s 2> /dev/null

gcc -g -o $TESTDIR/testmain $TESTDIR/main.c $TESTDIR/program.s
#$TESTDIR/testmain

exit 0

