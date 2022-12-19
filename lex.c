#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "syntax.h"

#define ADV(ctx) ++(ctx)->nextcol, (ctx)->c = fgetc((ctx)->stream)

typedef struct Lexer L;

static void
lexbcom(L *ctx)
{
	char chr, prev = ' ';
	for (;;) {
		chr = ctx->c;
		ADV(ctx);
		switch (chr) {
		case '/': if (prev == '*') return; break;
		case '\n': ++ctx->nextrow, ctx->nextcol = 1; break;
		case EOF: cerror("Block comment is not closed before the end of the source file.");
		}
		prev = chr;
	}
}

static int
atnum(L *ctx)
{
	return ctx->c >= '0' && ctx->c <= '9';
}

static int
lexnum(L *ctx)
{
	long long num = 0;
	do {
		num *= 10;
		num += ctx->c - '0';
		ADV(ctx);
	} while (atnum(ctx));
	ctx->yylval.num = num;
	return LITERAL;
}

static int
atsym(L *ctx)
{
	return (ctx->c >= 'a' && ctx->c <= 'z')
		|| (ctx->c >= 'A' && ctx->c <= 'Z')
		|| (ctx->c >= '0' && ctx->c <= '9')
		|| ctx->c == '_';
}

static int
lexsym(L *ctx)
{
	char *sym = ctx->symbuf;
	int len = 0;

	do {
		sym[len++] = ctx->c;
		ADV(ctx);
	} while (atsym(ctx) && len < 99);
	sym[len] = 0;
	
	while (atsym(ctx)) ADV(ctx);

	switch (sym[0]) {
	case 'c':
		if (!strcmp(sym, "char"))  return KCHAR;
		if (!strcmp(sym, "const")) return KCONST;
		break;
	case 'd':
		if (!strcmp(sym, "do")) return KDO;
		if (!strcmp(sym, "double")) return KDOUBLE;
		break;
	case 'e':
		if (!strcmp(sym, "else")) return KELSE;
		break;
	case 'f':
		if (!strcmp(sym, "float")) return KFLOAT;
		break;
	case 'i':
		if (!strcmp(sym, "if"))  return KIF;
		if (!strcmp(sym, "int")) return KINT;
		break;
	case 'l':
		if (!strcmp(sym, "long")) return KLONG;
		break;
	case 's':
		if (!strcmp(sym, "short"))  return KSHORT;
		if (!strcmp(sym, "signed")) return KSIGNED;
		break;
	case 'u':
		if (!strcmp(sym, "unsigned")) return KUNSIGNED;
		break;
	case 'v':
		if (!strcmp(sym, "void")) return KVOID;
		if (!strcmp(sym, "volatile")) return KVOLATILE;
		break;
	case 'w':
		if (!strcmp(sym, "while")) return KWHILE;
		break;
	}
	
	ctx->yylval.sym = sym;
	return SYMBOL;
}

static int
yylex(L *ctx)
{
	char f;
	for (;;) {
		ctx->yylval.sloc.file = ctx->curfile;
		ctx->yylval.sloc.row  = ctx->nextrow;
		ctx->yylval.sloc.col  = ctx->nextcol;
		
		switch (ctx->c) {
		
		case EOF: return 0;

		case ' ': case '\t': case '\r': ADV(ctx); break;
		
		case '\n':
			ADV(ctx);
			++ctx->nextrow, ctx->nextcol = 1;
			break;

		case '+': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return PLUSEQ;
			case '+': ADV(ctx); return PLUS2;
			default: return '+';
			}
		case '-': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return MINUSEQ;
			case '-': ADV(ctx); return MINUS2;
			default: return '-';
			}
		case '*': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return STAREQ;
			default: return '*';
			}
		case '/': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return SLASHEQ;
			case '/': ADV(ctx); while (ctx->c != EOF && ctx->c != '\n') ADV(ctx); break;
			case '*': ADV(ctx); lexbcom(ctx); break;
			default: return '/';
			}
			break;
		case '%': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return PERCEQ;
			default: return '%';
			}

		case '&': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return ANDEQ;
			case '&': ADV(ctx); return AND2;
			default: return '&';
			}
		case '^': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return HATEQ;
			default: return '^';
			}
		case '|': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return OREQ;
			case '|': ADV(ctx); return OR2;
			default: return '|';
			}

		case '!': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return NOTEQ;
			default: return '!';
			}
		case '=': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return EQ2;
			default: return '=';
			}
		case '<': ADV(ctx);
			switch (ctx->c) {
			case '<': ADV(ctx);
				if (ctx->c == '=') { ADV(ctx); return LT2EQ; }
				else { return LT2; }
			case '=': ADV(ctx); return LTEQ;
			default: return '<';
			}
		case '>': ADV(ctx);
			switch (ctx->c) {
			case '>': ADV(ctx);
				if (ctx->c == '=') { ADV(ctx); return GT2EQ; }
				else { return GT2; }
			case '=': ADV(ctx); return GTEQ;
			default: return '>';
			}

		case '(': case ')': case '{': case '}': case ';': case ',': case '~': case '#':
			f = ctx->c; ADV(ctx); return f;

		default:
			if (atnum(ctx)) return lexnum(ctx);
			if (atsym(ctx)) return lexsym(ctx);
			/* TODO report error location! */
			cerror("Unexpected character %c.", ctx->c);
		}
	}
}

Token
lextok(L *ctx)
{
	ctx->yylval.kind = yylex(ctx);
	return ctx->yylval;
}

void
initlex(L *ctx, char *path, void *file)
{
	ctx->stream = file;
	ctx->curfile = path;
	ADV(ctx);
	ctx->nextrow = 1;
	ctx->nextcol = 1;
}

