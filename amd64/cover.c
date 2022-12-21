#include <stdlib.h>

#include "../util.h"
#include "../ast.h"
#include "ins.h"

struct ins *cover(EXPR expr);

static struct ins *
cover_literal(struct ast_literal *literal)
{
	struct ins *ins = calloc(1, sizeof *ins);
	ins->opcode = OPC_MOV_RI;
	ins->embedded_reg = true;
	ins->has_immed = true;
	ins->immed = literal->value;
	return ins;
}

static struct ins *
cover_unop(struct ast_unop *unop)
{
	switch (unop->op) {
	default: return NULL;
	}
}

static struct ins *
cover_binop(struct ast_binop *binop)
{
	struct ins *ins;
	switch (binop->op) {
	case '+':
		ins = calloc(1, sizeof *ins + 2 * sizeof(struct ins *));
		ins->opcode = OPC_ADD_RR;
		ins->has_modrm = true;
		ins->mod = MOD_REG;
		ins->rm  = 0;
		ins->reg = 1;
		ins->arity = 2;
		ins->operands[0] = cover(binop->lhs);
		ins->operands[1] = cover(binop->rhs);
		return ins;
	case '-':
		ins = calloc(1, sizeof *ins + 2 * sizeof(struct ins *));
		ins->opcode = OPC_SUB_RR;
		ins->has_modrm = true;
		ins->mod = MOD_REG;
		ins->rm  = 0;
		ins->reg = 1;
		ins->arity = 2;
		ins->operands[0] = cover(binop->lhs);
		ins->operands[1] = cover(binop->rhs);
		return ins;
	default: return NULL;
	}
}

struct ins *
cover(EXPR expr)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_LITERAL: return cover_literal(expr);
	case EXPR_UNOP:    return cover_unop   (expr);
	case EXPR_BINOP:   return cover_binop  (expr);
	default: return NULL;
	}
}

