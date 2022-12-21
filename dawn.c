#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "dawn.h"
#include "syntax.h"

int
main(int argc, char **argv)
{
	Lexer  lexer;
	Parser parser;
	initlex(&lexer, "stdin", stdin);
	parser.lexer = &lexer;
	parse(&parser);
	return 0;
}

