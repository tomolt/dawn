#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"

int
main(int argc, char **argv)
{
	Parser parser;
	initlex(&parser, "stdin", stdin);
	parse(&parser);
	return 0;
}

