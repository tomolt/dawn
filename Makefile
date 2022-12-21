CC=gcc
LD=gcc
CPPFLAGS=-D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g
LDFLAGS=-g

.PHONY: all clean

all: dawn

clean:
	rm -f *.o
	rm -f dawn

dawn: dawn.o util.o error.o lex.o parse.o scope.o
	$(LD) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dawn.o: dawn.h
util.o: util.h
lex.o: util.h syntax.h
parse.o: util.h syntax.h
scope.o: util.h syntax.h
