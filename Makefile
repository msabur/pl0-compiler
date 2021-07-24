CC=gcc
CFLAGS=-Wall -Wextra

SRCS=lex.c parser.c compiler.c
OBJS=$(SRCS:.c=.o)

.PHONY: clean

all: compiler vm .depend

compiler: $(OBJS)

# to recompile a file when one of its header files change
.depend: $(SRCS)
	$(CC) -MM $^ > $@

include .depend

clean:
	$(RM) compiler vm *.o output.pm0
