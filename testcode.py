import os
import sys
import difflib

def check_code():
    testcase = sys.argv[2]
    testname = os.path.basename(testcase).replace("code", "0")

    actual =list(filter(
            lambda line: '.globl' not in line,
            map(lambda x: x.replace("\t", ""), sys.stdin.readlines())
    ))

    with open(sys.argv[2]) as fh:
        expected = fh.readlines()

    diff = difflib.Differ().compare(expected, actual)

    if actual == expected:
        print("OK {}".format(testname))
    else:
        print("FF {}".format(testname))
        print("".join(diff))



if __name__ == '__main__':
    mode = sys.argv[1]

    if mode == "--check-code":
        check_code()

