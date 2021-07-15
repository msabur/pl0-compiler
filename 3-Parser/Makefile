CC=gcc
CFLAGS=-g -Wall

# our primary source file
MAIN=parser.c

# our helper source files
AUX=lex.c driver.c

SRCS=$(MAIN) $(AUX)
OBJS=$(SRCS:.c=.o)

# name of generated executable
BIN=a.out

.PHONY: all clean

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(BIN) *.o

