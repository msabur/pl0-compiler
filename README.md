# Pl/0 Compiler Project

A compiler for the PL/0 language, made for the Systems Software class at UCF in Spring 2021.

# Authors:

- [Grant-Allan](https://github.com/Grant-Allan)
- [msabur](https://github.com/msabur)

# Usage:

First, build:
```
make
```

Then, give it a PL/0 program:
```
./compiler [options] filename
```
Example:
```
./compiler fibonacci.pl0
```
Finally, run the compiled program:
```
./vm output.pm0
```
<hr> 

The compiler accepts these options:
```
-c --show-code                      - print generated code
-l --show-lexemes                   - print lexeme table
-o --output=<FILE>                  - the output file (default: output.pm0)
-n --no-output                      - don't make output file
```
