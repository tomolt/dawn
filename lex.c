#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "syntax.h"

#define ADV(ctx) (++(ctx)->nextcol, (ctx)->c = fgetc((ctx)->stream))

typedef struct Parser P;

static void
lex_block_comment(P *ctx)
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
at_number(P *ctx)
{
	return ctx->c >= '0' && ctx->c <= '9';
}

static int
lex_number(P *ctx)
{
	int64_t num = 0;
	do {
		num *= 10;
		num += ctx->c - '0';
		ADV(ctx);
	} while (at_number(ctx));
	ctx->token.num = num;
	return LITERAL;
}

static int
at_symbol(P *ctx)
{
	return (ctx->c >= 'a' && ctx->c <= 'z')
		|| (ctx->c >= 'A' && ctx->c <= 'Z')
		|| (ctx->c >= '0' && ctx->c <= '9')
		|| ctx->c == '_';
}

static int
lex_symbol(P *ctx)
{
	char *sym = ctx->symbuf;
	int len = 0;

	do {
		sym[len++] = ctx->c;
		ADV(ctx);
	} while (at_symbol(ctx) && len < 99);
	sym[len] = 0;
	
	while (at_symbol(ctx)) ADV(ctx);

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

int
lex_token(P *ctx)
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
			case '*': ADV(ctx); lex_block_comment(ctx); break;
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
			if (at_number(ctx)) return lex_number(ctx);
			if (at_symbol(ctx)) return lex_symbol(ctx);
			/* TODO report error location! */
			cerror("Unexpected character %c.", ctx->c);
		}
	}
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
