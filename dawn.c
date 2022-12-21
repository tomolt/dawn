#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"
#include "ast.h"

struct ins;

extern struct ins *cover(EXPR expr);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	parse(&parser);
	return 0;
}

