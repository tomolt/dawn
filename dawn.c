#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dawn.h"
#include "syntax.h"
#include "ast.h"

struct tile;

extern struct tile *dawn_cover_ast(STMT stmt);
extern void dawn_sethi_ullman(struct tile *tile, void *stream);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	STMT stmt = parse(&parser);
	struct tile *tile = dawn_cover_ast(stmt);
	dawn_sethi_ullman(tile, stdout);
	return 0;
}

