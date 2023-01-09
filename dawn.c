#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dawn.h"
#include "syntax.h"
#include "muop.h"

void compile(const struct museq *museq, void *file);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	struct museq museq = { 0 };
	Parser parser = { 0 };
	parser.museq = &museq;
	initlex(&parser, "stdin", stdin);
	parse(&parser);
	museq_format(&museq, stderr);
	compile(&museq, stdout);
	free(museq.muops);
	return 0;
}

