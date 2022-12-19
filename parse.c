#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "ir.h"
#include "lang.h"
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
typedef struct Dtor Dtor;

struct Nud {
	uint code : 8;
};

struct Led {
	uint bp : 4;
	uint code : 4;
};

struct Dtor {
	Type *type;
	char *sym;
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

static Type *
pbase(P *ctx)
{
	uint qual = 0;
	int sign = -1;
	int rank = -1;
	int kind = -1;
	bool commited = false;
	
	for (;;) {
		switch (ctx->tok.kind) {
		case KCONST:
			qual |= QCONST;
			break;

		case KVOLATILE:
			qual |= QVOLATILE;
			break;
		
		case KVOID:
			if (kind >= 0) error(ctx->tok.sloc, "You cannot combine different base types.");
			kind = TVOID;
			break;

		case KINT:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			kind = TINT;
			break;

		case KCHAR:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			if (rank >= 0) error(ctx->tok.sloc, "Size of integer type is over-specified.");
			kind = TINT;
			rank = RCHAR;
			break;

		case KSHORT:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			if (rank >= 0) error(ctx->tok.sloc, "Size of integer type is over-specified.");
			kind = TINT;
			rank = RSHORT;
			break;

		case KLONG:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			kind = TINT;
			if (rank == RLONG) {
				rank = RLLONG;
			} else {
				if (rank >= 0) error(ctx->tok.sloc, "Size of integer type is over-specified.");
				rank = RLONG;
			}
			break;

		case KSIGNED:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			if (sign >= 0) error(ctx->tok.sloc, "Signedness of integer type is over-specified.");
			kind = TINT;
			sign = 1;
			break;

		case KUNSIGNED:
			if (kind >= 0 && kind != TINT) error(ctx->tok.sloc, "You cannot combine different base types.");
			if (sign >= 0) error(ctx->tok.sloc, "Signedness of integer type is over-specified.");
			kind = TINT;
			sign = 0;
			break;

		case KFLOAT:
			if (kind >= 0) error(ctx->tok.sloc, "You cannot combine different base types.");
			kind = TFLOAT;
			rank = RFLOAT;
			break;
		
		case KDOUBLE:
			if (kind >= 0) error(ctx->tok.sloc, "You cannot combine different base types.");
			kind = TFLOAT;
			rank = RDOUBLE;
			break;

		default:
			if (!commited) return NULL;
			switch (kind) {
			case TVOID:
				return voidtype(qual);
			case TINT:
				if (rank < 0) rank = RINT;
				if (sign < 0) sign = 1;
				return inttype(rank, sign, qual);
			case TFLOAT:
				return floattype(rank, qual);
			default:
				error(ctx->tok.sloc, "Missing primitive type.");
			}
		}
		ADV(ctx);
		commited = true;
	}
}

static Dtor
pdtor(P *ctx, Type *base)
{
	Dtor dtor;
	uint qual;

	dtor.type = base;
	while (ctx->tok.kind == '*') {
		ADV(ctx);
		qual = 0;
		while (ctx->tok.kind == KCONST || ctx->tok.kind == KVOLATILE) {
			switch (ctx->tok.kind) {
			case KCONST: qual |= QCONST; break;
			case KVOLATILE: qual |= QVOLATILE; break;
			}
			ADV(ctx);
		}
		dtor.type = ptrtype(dtor.type, qual);
	}
	
	if (ctx->tok.kind != SYMBOL) error(ctx->tok.sloc, "Expected symbol name in declaration.");
	dtor.sym = ctx->tok.sym;
	ADV(ctx);
	
	if (ctx->tok.kind == '(') {
		ADV(ctx);
		skip(ctx, ')');
		dtor.type = functype(dtor.type);
	}
	
	return dtor;
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

#if 0
static void
pwhile(P *ctx)
{
	Expr cond;
	uint entry, header, bodyin, bodyout, exit;
	
	entry = CurBlk;
	skip(ctx, KWHILE);

	header = newblk();
	skip(ctx, '(');
	cond = pexpr(ctx, 0);
	skip(ctx, ')');
	cccjmp(torval(cond));
	
	bodyin = newblk();
	pstmt(ctx);
	bodyout = CurBlk;
	exit = newblk();
	
	linkb(entry, header);
	linkb(header, exit);
	linkb(header, bodyin);
	linkb(bodyout, header);
}

static void
pifelse(P *ctx)
{
	Expr cond;
	uint entry, exit;
	uint truein, trueout;
	uint falsein, falseout;
	
	entry = CurBlk;
	skip(ctx, KIF);
	
	skip(ctx, '(');
	cond = pexpr(ctx, 0);
	skip(ctx, ')');
	cccjmp(torval(cond));

	truein = newblk();
	pstmt(ctx);
	trueout = CurBlk;

	if (ctx->tok.kind == KELSE) {
		ADV(ctx);

		falsein = newblk();
		pstmt(ctx);
		falseout = CurBlk;
		exit = newblk();

		linkb(entry, falsein);
		linkb(entry, truein);
		linkb(falseout, exit);
		linkb(trueout,  exit);
	} else {
		exit = newblk();

		linkb(entry, exit);
		linkb(entry, truein);
		linkb(trueout, exit);
	}
}
#endif

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
	case KDO:    pdowhile(ctx); break;
	//case KWHILE: pwhile(ctx);   break;
	//case KIF:    pifelse(ctx);  break;
	case '{':    pblock(ctx);   break;
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

