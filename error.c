#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "syntax.h"

static const char *TokNames[NUMTOKS] = {
	[0]         = "end of input",
	[LITERAL]   = "number",
	[SYMBOL]    = "symbol",
	[KCHAR]     = "char",
	[KSHORT]    = "short",
	[KINT]      = "int",
	[KLONG]     = "long",
	[KCONST]    = "const",
	[KVOLATILE] = "volatile",
	[KDO]       = "do",
	[KWHILE]    = "while",
	[KIF]       = "if",
	[KELSE]     = "else",
	[PLUS2]     = "++",
	[MINUS2]    = "--",
	[OR2]       = "||",
	[AND2]      = "&&",
	[EQ2]       = "==",
	[NOTEQ]     = "!=",
	[LTEQ]      = "<=",
	[GTEQ]      = ">=",
	[LT2]       = "<<",
	[GT2]       = ">>",
	[PLUSEQ]    = "+=",
	[MINUSEQ]   = "-=",
	[STAREQ]    = "*=",
	[SLASHEQ]   = "/=",
	[PERCEQ]    = "%=",
	[LT2EQ]     = "<<=",
	[GT2EQ]     = ">>=",
	[ANDEQ]     = "&=",
	[HATEQ]     = "^=",
	[OREQ]      = "|=",
};

const char *
fmttok(Token tok)
{
	static char buf[2];
	if (TokNames[tok.kind]) return TokNames[tok.kind];
	buf[0] = (char) tok.kind;
	buf[1] = 0;
	return buf;
}

void
cerror(const char *fmt, ...)
{
	va_list va;
	fprintf(stderr, "error: ");
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(0);
}

void
error(SLoc sloc, const char *fmt, ...)
{
	va_list va;
	fprintf(stderr, "%s:%lu:%lu: ", sloc.file, sloc.row, sloc.col);
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(0);
}

