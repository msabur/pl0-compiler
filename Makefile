CFLAGS=-g
SOURCES=lex.c parser.c compiler.c
OUTPUTS=compiler vm

.PHONY: clean

all: $(OUTPUTS)

compiler: $(SOURCES) compiler.h

clean:
	$(RM) $(OUTPUTS) output.pm0
