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

dawn: dawn.o util.o error.o lex.o parse.o amd64/ins.o amd64/cover.o amd64/su.o
	$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dawn.o: dawn.h syntax.h ast.h
util.o: util.h
error.o: syntax.h
lex.o: syntax.h
parse.o: syntax.h ast.h
amd64/ins.o: amd64/ins.h
amd64/cover.o: syntax.h ast.h amd64/ins.h
amd64/su.o: amd64/ins.h

