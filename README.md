# Pl/0 Compiler Project

UCF COP3402 project to make a PL/0 compiler

# Authors:

- <a href="https://github.com/Grant-Allan">Grant Allan</a>
- <a href="https://github.com/msabur">Maahee Abdus Sabur</a>

# Usage:

First, build:
```bash
make
```

Then, run the compiler:
```bash
./compiler [input-file]
```
Or to see the generated code on screen, run it like this:
```bash
./compiler [input-file] -c
```

Finally, run the compiled program:
```bash
./vm output.pm0
```
