typedef struct expr_base *EXPR;
typedef struct stmt_base *STMT;

#define EXPR_KIND(expr) ((expr)->kind)
#define STMT_KIND(stmt) ((stmt)->kind)

enum {
	EXPR_literal,
	EXPR_varref,
	EXPR_unop,
	EXPR_binop,
};

struct expr_base {
	int kind;
};

struct ast_literal {
	struct expr_base base;
	int64_t value;
};

struct ast_varref {
	struct expr_base base;
	int id;
};

struct ast_unop {
	struct expr_base base;
	int  op;
	EXPR arg;
};

struct ast_binop {
	struct expr_base base;
	int  op;
	EXPR lhs;
	EXPR rhs;
};

enum {
	STMT_vardecl,
	STMT_exprstmt,
	STMT_ifelse,
};

struct stmt_base {
	int kind;
};

struct ast_vardecl {
	struct stmt_base base;
	int id;
};

struct ast_exprstmt {
	struct stmt_base base;
	EXPR expr;
};

struct ast_ifelse {
	struct stmt_base base;
	EXPR cond;
	STMT tbranch;
	STMT fbranch;
};

