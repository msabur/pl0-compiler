.PHONY: clean
all: compiler vm
compiler: compiler.c getopt.c lex.c parser.c 
clean:
	$(RM) compiler vm
