# Solution for the TUW course Compiler Construction

This compiler is the result of the compiler construction course at the Vienna University of Technology in the summer semester of 2015.

The goal was to implement a small functional language with closures in assembler. The repository contains multiple directories, 
each containing different completion states of the compiler (beginning with the lexer and ending with a full chain ending with 
code generation).

Some tests can be found at https://github.com/kkris/uebersetzerbau_tests

Due to the age of some tools used the code contains some hacks which allow different optimizations. 
The generated code is tries to minimize the number of instructions used (this was a bonus objective in the course) and thus mostly
tries to avoid redundant moves, tagging und untagging of values and register spills.
