typedef void *EXPR;
typedef void *STMT;

#define EXPR_KIND(expr) (*(int *)(expr))
#define STMT_KIND(stmt) (*(int *)(stmt))

enum {
	EXPR_LITERAL,
	EXPR_VARREF,
	EXPR_UNOP,
	EXPR_BINOP,
	EXPR_IFELSE,
};

struct ast_literal {
	int     kind;
	int64_t value;
};

struct ast_varref {
	int kind;
	int id;
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

struct ast_ifelse {
	int  kind;
	EXPR cond;
	EXPR tbranch;
	EXPR fbranch;
};

enum {
	STMT_VARDECL,
	STMT_EXPRSTMT,
};

struct ast_vardecl {
	int kind;
	int id;
};

struct ast_exprstmt {
	int kind;
	EXPR *expr;
};

