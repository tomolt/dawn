#include <stdlib.h>

#include "../util.h"
#include "../syntax.h"
#include "../ast.h"
#include "ins.h"

struct tile *cover(EXPR expr);

static struct tile *
cover_literal(struct ast_literal *literal)
{
	struct tile *tile = calloc(1, sizeof *tile);
	tile->opclass = OPCL_MOV_EI;
	tile->immed = literal->value;
	return tile;
}

static bool
cover_immed(EXPR expr, struct tile *tile)
{
	if (EXPR_KIND(expr) != EXPR_LITERAL) {
		return false;
	}
	tile->immed = ((struct ast_literal *)expr)->value;
	return true;
}

static struct tile *
cover_varref(struct ast_varref *ref)
{
	struct tile *tile = calloc(1, sizeof *tile);
	tile->opclass = OPCL_MOV_RM;
	tile->immed = 8 * ref->id;
	return tile;
}

static struct tile *
cover_unop(struct ast_unop *unop)
{
	struct tile *tile;
	switch (unop->op) {
	case '~':
		tile = calloc(1, sizeof *tile + 1 * sizeof(struct operand));
		tile->opclass = OPCL_MUL_M;
		tile->opnum   = OPNO_NOT;
		tile->arity   = 1;
		tile->operands[0] = (struct operand){ cover(unop->arg), SLOT_RM };
		return tile;
	case '-':
		tile = calloc(1, sizeof *tile + 1 * sizeof(struct operand));
		tile->opclass = OPCL_MUL_M;
		tile->opnum   = OPNO_NEG;
		tile->arity   = 1;
		tile->operands[0] = (struct operand){ cover(unop->arg), SLOT_RM };
		return tile;
	default: return NULL;
	}
}

static struct tile *
cover_binop(struct ast_binop *binop)
{
	struct tile *tile;
	switch (binop->op) {
	case '+': case '-': case '&': case '|': case '^':
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct operand));
		switch (binop->op) {
		case '+': tile->opnum = OPNO_ADD; break;
		case '-': tile->opnum = OPNO_SUB; break;
		case '&': tile->opnum = OPNO_AND; break;
		case '|': tile->opnum = OPNO_OR;  break;
		case '^': tile->opnum = OPNO_XOR; break;
		}
		if (cover_immed(binop->rhs, tile)) {
			tile->opclass = OPCL_ARITH_MI;
			tile->arity = 1;
			tile->operands[0] = (struct operand){ cover(binop->lhs), SLOT_RM };
		} else {
			tile->opclass = OPCL_ARITH_RM;
			tile->arity = 2;
			tile->operands[0] = (struct operand){ cover(binop->lhs), SLOT_REG };
			tile->operands[1] = (struct operand){ cover(binop->rhs), SLOT_RM };
		}
		return tile;

	case LT2: case GT2:
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct operand));
		switch (binop->op) {
		case LT2: tile->opnum = OPNO_SHL; break;
		case GT2: tile->opnum = OPNO_SAR; break;
		}
		if (cover_immed(binop->rhs, tile)) {
			tile->opclass = OPCL_SHIFT_MI;
			tile->arity = 1;
			tile->operands[0] = (struct operand){ cover(binop->lhs), SLOT_RM };
		} else {
			tile->opclass = OPCL_SHIFT_MC;
			tile->arity = 2;
			tile->operands[0] = (struct operand){ cover(binop->lhs), SLOT_RM };
			tile->operands[1] = (struct operand){ cover(binop->rhs), SLOT_NIL };
		}
		return tile;

	default: return NULL;
	}
}

struct tile *
cover_ifelse(struct ast_ifelse *ifelse)
{
	(void)ifelse;
	return NULL;
}

struct tile *
cover(EXPR expr)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_LITERAL: return cover_literal(expr);
	case EXPR_VARREF:  return cover_varref (expr);
	case EXPR_UNOP:    return cover_unop   (expr);
	case EXPR_BINOP:   return cover_binop  (expr);
	case EXPR_IFELSE:  return cover_ifelse (expr);
	default: return NULL;
	}
}

