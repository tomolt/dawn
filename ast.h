typedef void *EXPR;
typedef void *STMT;

#define EXPR_KIND(expr) (*(int *)(expr))
#define STMT_KIND(stmt) (*(int *)(stmt))

enum {
	EXPR_literal,
	EXPR_varref,
	EXPR_unop,
	EXPR_binop,
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

enum {
	STMT_vardecl,
	STMT_exprstmt,
	STMT_ifelse,
};

struct ast_vardecl {
	int kind;
	int id;
};

struct ast_exprstmt {
	int kind;
	EXPR expr;
};

struct ast_ifelse {
	int  kind;
	EXPR cond;
	STMT tbranch;
	STMT fbranch;
};

