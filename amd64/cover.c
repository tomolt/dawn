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
cover_unop(struct ast_unop *unop)
{
	switch (unop->op) {
	default: return NULL;
	}
}

static struct tile *
cover_binop(struct ast_binop *binop)
{
	struct tile *tile;
	switch (binop->op) {
	case '+':
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct tile *));
		tile->opnum = OPNO_ADD;
		if (cover_immed(binop->rhs, tile)) {
			tile->opclass = OPCL_ARITH_MI;
			tile->arity = 1;
			tile->operands[0] = cover(binop->lhs);
		} else {
			tile->opclass = OPCL_ARITH_RM;
			tile->arity = 2;
			tile->operands[0] = cover(binop->lhs);
			tile->operands[1] = cover(binop->rhs);
		}
		return tile;

	case '-':
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct tile *));
		tile->opnum = OPNO_SUB;
		if (cover_immed(binop->rhs, tile)) {
			tile->opclass = OPCL_ARITH_MI;
			tile->arity = 1;
			tile->operands[0] = cover(binop->lhs);
		} else {
			tile->opclass = OPCL_ARITH_RM;
			tile->arity = 2;
			tile->operands[0] = cover(binop->lhs);
			tile->operands[1] = cover(binop->rhs);
		}
		return tile;

	case LT2:
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct tile *));
		tile->opnum = OPNO_SHL;
		if (cover_immed(binop->rhs, tile)) {
			tile->opclass = OPCL_SHIFT_MI;
			tile->arity = 1;
			tile->operands[0] = cover(binop->lhs);
			return tile;
		} else {
			struct tile *mvtile1 = calloc(1, sizeof *mvtile1 + 2 * sizeof(struct tile *));
			mvtile1->opclass = OPCL_XCHG_FM;
			mvtile1->opnum = REG_CX;
			mvtile1->arity = 1;
			mvtile1->operands[0] = cover(binop->rhs);

			tile->opclass = OPCL_SHIFT_MC;
			tile->arity = 2;
			tile->operands[0] = cover(binop->lhs);
			tile->operands[1] = mvtile1;

			struct tile *mvtile2 = calloc(1, sizeof *mvtile2 + 2 * sizeof(struct tile *));
			mvtile2->opclass = OPCL_XCHG_FM;
			mvtile2->opnum = REG_CX;
			mvtile2->arity = 1;
			mvtile2->operands[0] = tile;

			return mvtile2;
		}

	default: return NULL;
	}
}

struct tile *
cover(EXPR expr)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_LITERAL: return cover_literal(expr);
	case EXPR_UNOP:    return cover_unop   (expr);
	case EXPR_BINOP:   return cover_binop  (expr);
	default: return NULL;
	}
}

