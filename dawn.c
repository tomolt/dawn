#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dawn.h"
#include "syntax.h"
#include "muop.h"
#include "module.h"

void compile(const struct museq *museq, void *file);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	struct module module = { 0 };
	module.num_functions = 1;
	module.functions = calloc(1, sizeof *module.functions);
	Parser parser = { 0 };
	parser.module = &module;
	parser.museq = &module.functions[0].museq;
	initlex(&parser, "stdin", stdin);
	parse(&parser);
	museq_format(&module.functions[0].museq, stderr);
	compile(&module.functions[0].museq, stdout);
	free(module.functions[0].museq.muops);
	free(module.functions);
	return 0;
}

