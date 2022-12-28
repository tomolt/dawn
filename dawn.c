#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"
#include "ast.h"

struct tile;

extern struct tile *cover(STMT stmt);
void dawn_sethi_ullman(struct tile *tile, void *stream);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	STMT stmt = parse(&parser);
	struct tile *tile = cover(stmt);
	dawn_sethi_ullman(tile, stdout);
	return 0;
}

