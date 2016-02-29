# Solution for the TUW course Compiler Construction

This compiler is the result of the compiler construction course at the Vienna University of Technology in the summer semester of 2015.

This test file showcases the various language features: [tests/functional.0](tests/functional.0)

The goal was to implement a small functional language with closures in assembler. The repository contains multiple directories, 
each containing different completion states of the compiler (beginning with the lexer and ending with a full chain from lexing to code generation).

Tests for all features can be found in the test directory.

Due to the age of some tools used the code contains some hacks which allow different optimizations. 
The generated code tries to minimize the number of instructions used (this was a bonus objective in the course) and thus mostly
tries to avoid redundant moves, tagging und untagging of values and register spills.
