#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "syntax.h"
#include "ast.h"

#define ADV(ctx) lextok(ctx)

#define CNONE   0
#define CNUM    1
#define CVAR    2
#define CPAREN  3
#define CPREFIX 4
#define CIF     5

#define CLEFTASSOC  1
#define CRIGHTASSOC 2
#define CCOMMA      3

#define PREFIXPREC 100

typedef struct Parser P;
typedef struct Nud Nud;
typedef struct Led Led;

struct Nud {
	uint code : 8;
};

struct Led {
	uint bp : 4;
	uint code : 4;
};

static Nud Nuds[NUMTOKS] = {
	['(']     = { CPAREN },
	[LITERAL] = { CNUM },
	[SYMBOL]  = { CVAR },
	['!']     = { CPREFIX },
	['~']     = { CPREFIX },
	['-']     = { CPREFIX },
	[KIF]     = { CIF },
	//[PLUS2]   = { CPREFIX },
	//[MINUS2]  = { CPREFIX },
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
#if 0
	['<']     = { 10, CLEFTASSOC },
	[LTEQ]    = { 10, CLEFTASSOC },
	['>']     = { 10, CLEFTASSOC },
	[GTEQ]    = { 10, CLEFTASSOC },
	[EQ2]     = {  9, CLEFTASSOC },
	[NOTEQ]   = {  9, CLEFTASSOC },
	[AND2]    = {  5, CLEFTASSOC },
	[OR2]     = {  4, CLEFTASSOC },
	['=']     = {  2, CRIGHTASSOC },
	[PLUSEQ]  = {  2, CRIGHTASSOC },
	[MINUSEQ] = {  2, CRIGHTASSOC },
	[STAREQ]  = {  2, CRIGHTASSOC },
	[SLASHEQ] = {  2, CRIGHTASSOC },
	[PERCEQ]  = {  2, CRIGHTASSOC },
	[LT2EQ]   = {  2, CRIGHTASSOC },
	[GT2EQ]   = {  2, CRIGHTASSOC },
	[ANDEQ]   = {  2, CRIGHTASSOC },
	[HATEQ]   = {  2, CRIGHTASSOC },
	[OREQ]    = {  2, CRIGHTASSOC },
	[',']     = {  1, CRIGHTASSOC },
#endif
};

//static void pstmt(P *);
//static void pblock(P *);

static EXPR
newliteral(int64_t value)
{
	struct ast_literal *expr = calloc(1, sizeof *expr);
	expr->kind  = EXPR_LITERAL;
	expr->value = value;
	return expr;
}

static EXPR
newvarref(int id)
{
	struct ast_varref *expr = calloc(1, sizeof *expr);
	expr->kind = EXPR_VARREF;
	expr->id   = id;
	return expr;
}

static EXPR
newunop(int op, EXPR arg)
{
	struct ast_unop *expr = calloc(1, sizeof *expr);
	expr->kind = EXPR_UNOP;
	expr->op   = op;
	expr->arg  = arg;
	return expr;
}

static EXPR
newbinop(int op, EXPR lhs, EXPR rhs)
{
	struct ast_binop *expr = calloc(1, sizeof *expr);
	expr->kind = EXPR_BINOP;
	expr->op   = op;
	expr->lhs  = lhs;
	expr->rhs  = rhs;
	return expr;
}

static EXPR
newifelse(EXPR *cond, EXPR *tbranch, EXPR *fbranch)
{
	struct ast_ifelse *expr = calloc(1, sizeof *expr);
	expr->kind = EXPR_IFELSE;
	expr->cond = cond;
	expr->tbranch = tbranch;
	expr->fbranch = fbranch;
	return expr;
}

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
		expr = newliteral(ctx->token.num);
		ADV(ctx);
		break;

	case CVAR:
		expr = newvarref(ctx->nextvar++);
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
		expr = newunop(token.kind, pexpr(ctx, PREFIXPREC));
		break;
	
	case CIF:
		{
			ADV(ctx);
			EXPR *cond = pexpr(ctx, 0);
			skip(ctx, KTHEN);
			EXPR *tbranch = pexpr(ctx, 0);
			skip(ctx, KELSE);
			EXPR *fbranch = pexpr(ctx, 0);
			expr = newifelse(cond, tbranch, fbranch);
		}
		break;
	}

	while (Leds[ctx->token.kind].bp > minbp) {
		switch (Leds[ctx->token.kind].code) {
		case CLEFTASSOC:
			token = ctx->token;
			ADV(ctx);
			expr = newbinop(token.kind, expr, pexpr(ctx, Leds[token.kind].bp));
			break;

		case CRIGHTASSOC:
			token = ctx->token;
			ADV(ctx);
			expr = newbinop(token.kind, expr, pexpr(ctx, Leds[token.kind].bp-1));
			break;
		}
	}

	return expr;
}

#if 0
static void
pblock(P *ctx)
{
	uint scope;
	skip(ctx, '{');
	scope = getscope();
	while (ctx->token.kind != '}') {
		if (!ctx->token.kind) error(ctx->token.sloc, "Unclosed braces.");
		pstmt(ctx);
	}
	ADV(ctx);
	resetscope(scope);
}

static void
pstmt(P *ctx)
{
	switch (ctx->token.kind) {
	case '{': pblock(ctx); break;
	default:
		pexpr(ctx, 0);
		skip(ctx, ';');
	}
}
#endif

void *
parse(P *ctx)
{
#if 0
	uint scope = getscope();
	ADV(ctx);
	while (ctx->token.kind) {
		pstmt(ctx);
	}
	resetscope(scope);
#endif
	ADV(ctx);
	EXPR expr = pexpr(ctx, 0);
	return expr;
}

