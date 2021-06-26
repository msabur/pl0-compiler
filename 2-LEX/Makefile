CC=gcc
CFLAGS=-g -Wall

# our source files
SRCS=lex.c driver.c

OBJS=$(SRCS:.c=.o)

# name of generated executable
BIN=a.out

.PHONY: all clean

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(BIN) $(OBJS)

