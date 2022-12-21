CC=gcc
LD=gcc
CPPFLAGS=-D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g
LDFLAGS=-g

.PHONY: all clean

all: dawn

clean:
	rm -f *.o
	rm -f amd64/*.o
	rm -f dawn

dawn: dawn.o util.o error.o lex.o parse.o scope.o amd64/ins.o amd64/cover.o
	$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dawn.o: util.h dawn.h syntax.h ast.h
util.o: util.h
error.o: util.h syntax.h
lex.o: util.h syntax.h
parse.o: util.h syntax.h ast.h
scope.o: util.h syntax.h
amd64/ins.o: util.h amd64/ins.h
amd64/cover.o: util.h syntax.h amd64/ins.h
