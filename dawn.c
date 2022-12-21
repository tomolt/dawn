#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"
#include "ast.h"

struct tile;

extern struct tile *cover(EXPR expr);
extern void emit(struct tile *tile, void *stream);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	EXPR expr = parse(&parser);
	struct tile *tile = cover(expr);
	emit(tile, stdout);
	return 0;
}

