#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dawn.h"
#include "syntax.h"
#include "pool.h"
#include "ast.h"
#include "amd64/ins.h"

struct tile;

extern struct tile *dawn_cover_ast(struct tiler *ctx, STMT stmt);
extern void dawn_sethi_ullman(struct tile *tile, void *stream);

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	Parser parser;
	initlex(&parser, "stdin", stdin);
	STMT stmt = parse(&parser);
	struct tiler tiler = { NULL };
	struct tile *tile = dawn_cover_ast(&tiler, stmt);
	dawn_sethi_ullman(tile, stdout);
	pool_release(&parser.ast_pool);
	pool_release(&tiler.tile_pool);
	return 0;
}

