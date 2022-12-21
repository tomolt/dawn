#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"
#include "ast.h"

struct ins;

extern struct ins *cover(EXPR expr);
extern void emit(struct ins *ins, void *stream);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	EXPR expr = parse(&parser);
	struct ins *ins = cover(expr);
	emit(ins, stdout);
	return 0;
}

