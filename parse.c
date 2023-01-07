#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "syntax.h"
#include "muop.h"

#define ADV(ctx) ((ctx)->token.kind = lex_token(ctx))

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

static size_t
construct_literal(P *ctx, int64_t value)
{
	// FIXME this will break for large literals.
	return museq_append_imm(ctx->museq, (int32_t)value);
}

static size_t
construct_varref(P *ctx, int var)
{
	return museq_append(ctx->museq, MU_LDL, var, 0);
}

static size_t
construct_unop(P *ctx, unsigned char token, size_t arg)
{
	uint8_t op;
	switch (token) {
	case '-': op = MU_NEG; break;
	case '~': op = MU_NOT; break;
	default: assert(0);
	}
	return museq_append(ctx->museq, op, arg, 0);
}

static size_t
construct_binop(P *ctx, unsigned char token, size_t lhs, size_t rhs)
{
	uint8_t op;
	switch (token) {
	case '+': op = MU_ADD; break;
	case '-': op = MU_SUB; break;
	case '&': op = MU_AND; break;
	case '|': op = MU_OR;  break;
	case '^': op = MU_XOR; break;
	case '*': op = MU_MUL; break;
	case '/': op = MU_DIV; break;
	case '%': op = MU_MOD; break;
	case LT2: op = MU_SHL; break;
	case GT2: op = MU_SHR; break;
	default: assert(0);
	}
	return museq_append(ctx->museq, op, lhs, rhs);
}

static void
skip(P *ctx, int kind)
{
	if (ctx->token.kind != kind) {
		error(ctx->token.sloc, "Unexpected %s.", fmttok(ctx->token));
	}
	ADV(ctx);
}

static size_t
pexpr(P *ctx, int minbp)
{
	size_t expr;
	Token token;

	switch (Nuds[ctx->token.kind].code) {
	case CNONE:
		error(ctx->token.sloc, "%s is not an expression.", fmttok(ctx->token));
		break;

	case CNUM:
		expr = construct_literal(ctx, ctx->token.num);
		ADV(ctx);
		break;

	case CVAR:
		expr = construct_varref(ctx, ctx->nextvar++);
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
		expr = construct_unop(ctx, token.kind, pexpr(ctx, PREFIXPREC));
		break;
	}

	while (Leds[ctx->token.kind].bp > minbp) {
		size_t rhs;
		switch (Leds[ctx->token.kind].code) {
		case CLEFTASSOC:
			token = ctx->token;
			ADV(ctx);
			rhs = pexpr(ctx, Leds[token.kind].bp);
			expr = construct_binop(ctx, token.kind, expr, rhs);
			break;

		case CRIGHTASSOC:
			token = ctx->token;
			ADV(ctx);
			rhs = pexpr(ctx, Leds[token.kind].bp-1);
			expr = construct_binop(ctx, token.kind, expr, rhs);
			break;
		}
	}

	return expr;
}

static void
pstmt(P *ctx)
{
	switch (ctx->token.kind) {
#if 0
	case KIF:
		{
			ADV(ctx);
			EXPR cond = pexpr(ctx, 0);
			skip(ctx, KTHEN);
			STMT tbranch = pstmt(ctx);
			skip(ctx, KELSE);
			STMT fbranch = pstmt(ctx);
			NEW_STMT(ctx, stmt, ifelse, cond, tbranch, fbranch);
		}
		break;
#endif
	default:
		pexpr(ctx, 0);
		skip(ctx, ';');
	}
}

void
parse(P *ctx)
{
	ADV(ctx);
	pstmt(ctx);
}

