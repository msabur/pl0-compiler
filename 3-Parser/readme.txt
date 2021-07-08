COP 3402 - Systems Software
Summer 2021
Homework #3 (Parser - Code Generator)
Authors: Maahee, Grant Allan
Due: 7/15/2021

=========
# To compile

## With makefile:
make

## Manually:
gcc driver.c parser.c lex.o

## If that doesn't work:
gcc driver.c parser.c lex.c

=========
# To run
./<name of generated executable> <path to input file>

Example, assuming the generated executable is named a.out:
./a.out input.txt
