CC=gcc
LD=gcc
CPPFLAGS=-D_XOPEN_SOURCE=700
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g
LDFLAGS=-g

.PHONY: all clean

all: dawn

clean:
	rm -f *.o
	rm -f amd64/*.o
	rm -f dawn

dawn: dawn.o util.o pool.o error.o lex.o parse.o amd64/isel.o amd64/regalloc.o amd64/asm.o
	$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dawn.o: dawn.h syntax.h ast.h
util.o: util.h
pool.o: pool.h
error.o: syntax.h
lex.o: syntax.h
parse.o: syntax.h ast.h
amd64/isel.o: syntax.h ast.h amd64/ins.h
amd64/regalloc.o: amd64/ins.h
amd64/asm.o: amd64/ins.h
