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

static void
cover_modrm(EXPR expr, struct ins *ins)
{
	ins->has_modrm = true;
	ins->mod = MOD_REG;
	ins->rm = ins->arity++;
	ins->operands[ins->rm] = cover(expr);
}

static bool
cover_immed(EXPR expr, struct ins *ins)
{
	if (EXPR_KIND(expr) != EXPR_LITERAL) {
		return false;
	}
	ins->has_immed = true;
	ins->immed = ((struct ast_literal *)expr)->value;
	return true;
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
		if (cover_immed(binop->rhs, ins)) {
			ins->opcode = OPC_ADD_RI;
			ins->reg = 0; // add
			ins->has_modrm = true;
			ins->mod = MOD_REG;
			ins->rm = 0;
			ins->arity = 1;
			ins->operands[0] = cover(binop->lhs);
		} else {
			ins->opcode = OPC_ADD_RM;
			ins->reg = 0;
			ins->arity = 1;
			ins->operands[0] = cover(binop->lhs);
			cover_modrm(binop->rhs, ins);
		}
		return ins;
	case '-':
		ins = calloc(1, sizeof *ins + 2 * sizeof(struct ins *));
		if (cover_immed(binop->rhs, ins)) {
			ins->opcode = OPC_SUB_RI;
			ins->reg = 5; // sub
			ins->has_modrm = true;
			ins->mod = MOD_REG;
			ins->rm = 0;
			ins->arity = 1;
			ins->operands[0] = cover(binop->lhs);
		} else {
			ins->opcode = OPC_SUB_RM;
			ins->reg = 0;
			ins->arity = 1;
			ins->operands[0] = cover(binop->lhs);
			cover_modrm(binop->rhs, ins);
		}
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

