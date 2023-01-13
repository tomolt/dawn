struct module;
struct museq;

typedef struct SLoc   SLoc;
typedef struct Token  Token;
typedef struct Parser Parser;

/* All named tokens have to be in the ranges [1,32], [48,57], [65,90], or [97,122]
 * so they do not collide with ASCII punctuation characters that are interpreted
 * as tokens literally. */
enum {
	/* 0 is implicit end of input */
	/* [1,2] */
	LITERAL = 1, SYMBOL,
	/* [3,11] */
	/* [14,17] */
	KWHILE, KIF, KTHEN, KELSE,
	/* [18,27] */
	PLUS2, MINUS2,
	OR2, AND2, EQ2, NOTEQ, LTEQ, GTEQ,
	LT2, GT2,
	/* [65,74] */
	PLUSEQ = 65, MINUSEQ, STAREQ, SLASHEQ,
	PERCEQ, LT2EQ, GT2EQ, ANDEQ, HATEQ, OREQ,
	
	NUMTOKS = 127
};

struct SLoc {
	const char *file;
	size_t row;
	size_t col;
};

struct Token {
	unsigned char kind;
	SLoc sloc;
	union {
		int64_t num;
		char   *sym;
	};
};

struct Parser {
	void  *stream;
	int    c;
	char  *curfile;
	size_t nextrow;
	size_t nextcol;
	char   symbuf[100];
	Token  token;
	int    nextvar; // TEMP
	struct module *module;
	struct museq *museq;
};

const char *fmttok(Token);
void cerror(const char *, ...);
void error(SLoc, const char *, ...);

int lex_token(Parser *ctx);
void initlex(Parser *, char *, void *);
void parse(Parser *);

