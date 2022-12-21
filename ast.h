typedef void *EXPR;

#define EXPR_KIND(expr) ((int)(expr)->kind)

enum {
	EXPR_LITERAL,
	EXPR_UNOP,
	EXPR_BINOP,
};

struct ast_literal {
	int     kind;
	int64_t value;
};

struct ast_unop {
	int  kind;
	int  op;
	EXPR arg;
};

struct ast_binop {
	int  kind;
	int  op;
	EXPR lhs;
	EXPR rhs;
};

