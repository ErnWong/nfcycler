PREFIX ?= /usr/local
CC ?= cc
BIN = nfcycler

SRC = $(wildcard src/*.c)
SRC_EXAMPLES = $(wildcard examples/*.c)
OUT_EXAMPLES = $(SRC_EXAMPLES:.c=)
DEPS = $(wildcard deps/*/*.c)
OBJS = $(DEPS:.c=.o)

CFLAGS = -std=c99 -Ideps -Wall -Wextra -D_POSIX_C_SOURCE

all: $(BIN)

$(BIN): $(SRC) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJ)

install:
	install $(BIN) $(PREFIX)/bin

uninstall:
	rm $(PREFIX)/bin/$(BIN)

examples: $(OUT_EXAMPLES)

examples/%: examples/%.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONEY: all clean install uninstall examples
