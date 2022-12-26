#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "syntax.h"

#define ADV(ctx) ++(ctx)->nextcol, (ctx)->c = fgetc((ctx)->stream)

typedef struct Parser P;

static void
lexbcom(P *ctx)
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
atnum(P *ctx)
{
	return ctx->c >= '0' && ctx->c <= '9';
}

static int
lexnum(P *ctx)
{
	long long num = 0;
	do {
		num *= 10;
		num += ctx->c - '0';
		ADV(ctx);
	} while (atnum(ctx));
	ctx->token.num = num;
	return LITERAL;
}

static int
atsym(P *ctx)
{
	return (ctx->c >= 'a' && ctx->c <= 'z')
		|| (ctx->c >= 'A' && ctx->c <= 'Z')
		|| (ctx->c >= '0' && ctx->c <= '9')
		|| ctx->c == '_';
}

static int
lexsym(P *ctx)
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
	case 'e':
		if (!strcmp(sym, "else")) return KELSE;
		break;
	case 'i':
		if (!strcmp(sym, "if"))  return KIF;
		break;
	case 't':
		if (!strcmp(sym, "then")) return KTHEN;
		break;
	case 'w':
		if (!strcmp(sym, "while")) return KWHILE;
		break;
	}
	
	ctx->token.sym = sym;
	return SYMBOL;
}

static int
yylex(P *ctx)
{
	char f;
	for (;;) {
		ctx->token.sloc.file = ctx->curfile;
		ctx->token.sloc.row  = ctx->nextrow;
		ctx->token.sloc.col  = ctx->nextcol;
		
		switch (ctx->c) {
		
		case EOF: return 0;

		case ' ': case '\t': case '\r': ADV(ctx); break;
		
		case '\n':
			ADV(ctx);
			++ctx->nextrow, ctx->nextcol = 1;
			break;

		case '*': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return STAREQ;
			default: return '*';
			}
		case '%': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return PERCEQ;
			default: return '%';
			}
		case '^': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return HATEQ;
			default: return '^';
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
		case '&': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return ANDEQ;
			case '&': ADV(ctx); return AND2;
			default: return '&';
			}
		case '|': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return OREQ;
			case '|': ADV(ctx); return OR2;
			default: return '|';
			}

		case '/': ADV(ctx);
			switch (ctx->c) {
			case '=': ADV(ctx); return SLASHEQ;
			case '/': ADV(ctx); while (ctx->c != EOF && ctx->c != '\n') ADV(ctx); break;
			case '*': ADV(ctx); lexbcom(ctx); break;
			default: return '/';
			}
			break;

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
lextok(P *ctx)
{
	ctx->token.kind = yylex(ctx);
	return ctx->token;
}

void
initlex(P *ctx, char *path, void *file)
{
	ctx->stream = file;
	ctx->curfile = path;
	ctx->nextrow = 1;
	ctx->nextcol = 1;
	ctx->nextvar = 0;
	ADV(ctx);
}

