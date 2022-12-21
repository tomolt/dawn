#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "syntax.h"

#define ADV(ctx) (ctx)->tok = mxnexttok((ctx)->mx)

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
	[PLUS2]   = { CPREFIX },
	[MINUS2]  = { CPREFIX },
	['+']     = { CPREFIX },
	['-']     = { CPREFIX },
	['*']     = { CPREFIX },
	['&']     = { CPREFIX },
};

static Led Leds[NUMTOKS] = {
	['*']     = { 13, CLEFTASSOC },
	['/']     = { 13, CLEFTASSOC },
	['%']     = { 13, CLEFTASSOC },
	['+']     = { 12, CLEFTASSOC },
	['-']     = { 12, CLEFTASSOC },
	[LT2]     = { 11, CLEFTASSOC },
	[GT2]     = { 11, CLEFTASSOC },
	['<']     = { 10, CLEFTASSOC },
	[LTEQ]    = { 10, CLEFTASSOC },
	['>']     = { 10, CLEFTASSOC },
	[GTEQ]    = { 10, CLEFTASSOC },
	[EQ2]     = {  9, CLEFTASSOC },
	[NOTEQ]   = {  9, CLEFTASSOC },
	['&']     = {  8, CLEFTASSOC },
	['^']     = {  7, CLEFTASSOC },
	['|']     = {  6, CLEFTASSOC },
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
};

static void pstmt(P *);
static void pblock(P *);

static void
skip(P *ctx, int kind)
{
	if (ctx->tok.kind != kind) {
		error(ctx->tok.sloc, "Unexpected %s.", fmttok(ctx->tok));
	}
	ADV(ctx);
}

static Expr
pexpr(P *ctx, int minbp)
{
	Expr  expr;
	Token tok;

	switch (Nuds[ctx->tok.kind].code) {
	case CNONE:
		error(ctx->tok.sloc, "%s is not an expression.", fmttok(ctx->tok));
		break;

	case CNUM:
		expr = ccliteral(ctx->tok.num);
		ADV(ctx);
		break;

	case CVAR:
		expr = ccsymbol(ctx->tok.sym);
		ADV(ctx);
		break;

	case CPAREN:
		ADV(ctx);
		expr = pexpr(ctx, 0);
		skip(ctx, ')');
		break;

	case CPREFIX:
		tok = ctx->tok;
		ADV(ctx);
		expr = ccunop(tok, pexpr(ctx, PREFIXPREC));
		break;
	}

	while (Leds[ctx->tok.kind].bp > minbp) {
		switch (Leds[ctx->tok.kind].code) {
		case CLEFTASSOC:
			tok = ctx->tok;
			ADV(ctx);
			expr = ccinfix(tok, expr, pexpr(ctx, Leds[tok.kind].bp));
			break;

		case CRIGHTASSOC:
			tok = ctx->tok;
			ADV(ctx);
			expr = ccinfix(tok, expr, pexpr(ctx, Leds[tok.kind].bp-1));
			break;
		}
	}

	return expr;
}

static bool
pdecl(P *ctx)
{
	Type *base;
	Dtor dtor;

	base = pbase(ctx);
	if (!base) return false;
	
	dtor = pdtor(ctx, base);
	
	if (ctx->tok.kind == '{') {
		if (dtor.type->kind != TFUNC) error(ctx->tok.sloc, "Non-function declaration followed by code block.");
		if (CurProc != NO_PROC) error(ctx->tok.sloc, "You cannot nest function definitions.");
		newproc(dtor.sym);
		pblock(ctx);
		CurProc = NO_PROC;
		return true;
	} else {
		ccdecl(dtor.sym, dtor.type);

		if (ctx->tok.kind == '=') {
			ADV(ctx);
			pexpr(ctx, 1);
		}
		
		while (ctx->tok.kind == ',') {
			ADV(ctx);
			dtor = pdtor(ctx, base);
			ccdecl(dtor.sym, dtor.type);
			if (ctx->tok.kind == '=') {
				ADV(ctx);
				pexpr(ctx, 1);
			}
		}

		skip(ctx, ';');
		return true;
	}
}

static void
pdowhile(P *ctx)
{
	Expr cond;
	uint bodyin;
	
	bodyin = newlabel();

	skip(ctx, KDO);
	
	putins(INS_DI(ANCHOR, 0, NO_VAR, bodyin));
	pstmt(ctx);

	skip(ctx, KWHILE);
	skip(ctx, '(');
	cond = pexpr(ctx, 0);
	cccjmp(torval(cond), bodyin);
	skip(ctx, ')');
	skip(ctx, ';');
}

static void
pblock(P *ctx)
{
	uint scope;
	skip(ctx, '{');
	scope = getscope();
	while (ctx->tok.kind != '}') {
		if (!ctx->tok.kind) error(ctx->tok.sloc, "Unclosed braces.");
		pstmt(ctx);
	}
	ADV(ctx);
	resetscope(scope);
}

static void
pstmt(P *ctx)
{
	switch (ctx->tok.kind) {
	case KDO: pdowhile(ctx); break;
	case '{': pblock(ctx);   break;
	default:
		if (!pdecl(ctx)) {
			pexpr(ctx, 0);
			skip(ctx, ';');
		}
	}
}

void
parse(P *ctx)
{
	uint scope;
	
	CurProc = NO_PROC;
	scope = getscope();
	ADV(ctx);
	while (ctx->tok.kind) {
		if (!pdecl(ctx)) {
			error(ctx->tok.sloc, "The top level must consist purely of declarations/definitions.");
		}
	}
	resetscope(scope);
}

