CC=gcc
LD=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g
LDFLAGS=-g

.PHONY: all clean

all: dawn

clean:
	rm -f *.o
	rm -f dawn

dawn: dawn.o util.o error.o lex.o
	$(LD) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

dawn.o: dawn.h
util.o: util.h
lex.o: util.h syntax.h

