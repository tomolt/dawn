#include <stdint.h>
#include <stdlib.h>

#include "syntax.h"
#include "ast.h"
#include "pool.h"

#define ADV(ctx) ((ctx)->token.kind = lex_token(ctx))
#define NEW_EXPR(ctx, expr, name, ...) do {						\
		struct ast_##name *_n = pool_alloc(&(ctx)->ast_pool, sizeof *_n);	\
		*_n = (struct ast_##name){ EXPR_##name, __VA_ARGS__ };			\
		(expr) = _n;								\
	} while (0)
#define NEW_STMT(ctx, stmt, name, ...) do {						\
		struct ast_##name *_n = pool_alloc(&(ctx)->ast_pool, sizeof *_n);	\
		*_n = (struct ast_##name){ STMT_##name, __VA_ARGS__ };			\
		(stmt) = _n;								\
	} while (0)

#define CNONE   0
#define CNUM    1
#define CVAR    2
#define CPAREN  3
#define CPREFIX 4

#define CLEFTASSOC  1
#define CRIGHTASSOC 2
#define CCOMMA      3

#define PREFIXPREC 100

typedef struct Parser P;
typedef struct Nud Nud;
typedef struct Led Led;

struct Nud {
	unsigned code : 8;
};

struct Led {
	unsigned bp : 4;
	unsigned code : 4;
};

static Nud Nuds[NUMTOKS] = {
	['(']     = { CPAREN },
	[LITERAL] = { CNUM },
	[SYMBOL]  = { CVAR },
	['!']     = { CPREFIX },
	['~']     = { CPREFIX },
	['-']     = { CPREFIX },
};

static Led Leds[NUMTOKS] = {
	['*']     = { 10, CLEFTASSOC },
	['/']     = { 10, CLEFTASSOC },
	['%']     = { 10, CLEFTASSOC },
	[LT2]     = { 10, CLEFTASSOC },
	[GT2]     = { 10, CLEFTASSOC },
	['&']     = { 10, CLEFTASSOC },
	['+']     = {  8, CLEFTASSOC },
	['-']     = {  8, CLEFTASSOC },
	['|']     = {  8, CLEFTASSOC },
	['^']     = {  8, CLEFTASSOC },
};

static void
skip(P *ctx, int kind)
{
	if (ctx->token.kind != kind) {
		error(ctx->token.sloc, "Unexpected %s.", fmttok(ctx->token));
	}
	ADV(ctx);
}

static EXPR
pexpr(P *ctx, int minbp)
{
	EXPR  expr;
	Token token;

	switch (Nuds[ctx->token.kind].code) {
	case CNONE:
		error(ctx->token.sloc, "%s is not an expression.", fmttok(ctx->token));
		break;

	case CNUM:
		NEW_EXPR(ctx, expr, literal, ctx->token.num);
		ADV(ctx);
		break;

	case CVAR:
		NEW_EXPR(ctx, expr, varref, ctx->nextvar++);
		ADV(ctx);
		break;

	case CPAREN:
		ADV(ctx);
		expr = pexpr(ctx, 0);
		skip(ctx, ')');
		break;

	case CPREFIX:
		token = ctx->token;
		ADV(ctx);
		NEW_EXPR(ctx, expr, unop, token.kind, pexpr(ctx, PREFIXPREC));
		break;
	}

	while (Leds[ctx->token.kind].bp > minbp) {
		EXPR *rhs;
		switch (Leds[ctx->token.kind].code) {
		case CLEFTASSOC:
			token = ctx->token;
			ADV(ctx);
			rhs = pexpr(ctx, Leds[token.kind].bp);
			NEW_EXPR(ctx, expr, binop, token.kind, expr, rhs);
			break;

		case CRIGHTASSOC:
			token = ctx->token;
			ADV(ctx);
			rhs = pexpr(ctx, Leds[token.kind].bp-1);
			NEW_EXPR(ctx, expr, binop, token.kind, expr, rhs);
			break;
		}
	}

	return expr;
}

static STMT
pstmt(P *ctx)
{
	STMT stmt;
	switch (ctx->token.kind) {
	case KIF:
		{
			ADV(ctx);
			EXPR *cond = pexpr(ctx, 0);
			skip(ctx, KTHEN);
			STMT *tbranch = pstmt(ctx);
			skip(ctx, KELSE);
			STMT *fbranch = pstmt(ctx);
			NEW_STMT(ctx, stmt, ifelse, cond, tbranch, fbranch);
		}
		break;
	default:
		NEW_STMT(ctx, stmt, exprstmt, pexpr(ctx, 0));
		skip(ctx, ';');
	}
	return stmt;
}

void *
parse(P *ctx)
{
	ADV(ctx);
	return pstmt(ctx);
}

