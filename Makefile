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
	rm -f risc-v/*.o
	rm -f dawn

dawn: dawn.o util.o pool.o error.o lex.o parse.o muop.o compile.o risc-v/tile.o risc-v/asm.o
	$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dawn.o: dawn.h syntax.h module.h
util.o: util.h
pool.o: pool.h
error.o: syntax.h
lex.o: syntax.h
parse.o: syntax.h muop.h module.h
muop.o: muop.h
compile.o: compile.h revbuf.h
amd64/asm.o: amd64/ins.h
risc-v/tile.o: risc-v/ins.h muop.h risc-v/templates.def
risc-v/asm.o: risc-v/ins.h revbuf.h
