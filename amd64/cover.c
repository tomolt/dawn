#include <stdlib.h>
#include <limits.h>

#include "../util.h"
#include "../syntax.h"
#include "../ast.h"
#include "ins.h"

struct tile *cover(EXPR expr);

static struct tile *
cover_literal(struct ast_literal *literal)
{
	struct tile *tile = calloc(1, sizeof *tile);
	tile->ins.opcode = OPC_MOV_EI();
	tile->ins.immed = literal->value;
	tile->genesis = SLOT_EMB;
	return tile;
}

static bool
cover_immed(EXPR expr, struct tile *tile)
{
	if (EXPR_KIND(expr) != EXPR_LITERAL) {
		return false;
	}
	tile->ins.immed = ((struct ast_literal *)expr)->value;
	return true;
}

static void
cover_modrm(EXPR expr, bool is_dest, struct tile *tile)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_VARREF:
		{
			const struct ast_varref *ref = expr;
			tile->ins.mod = MOD_MEM_LD;
			tile->ins.rm  = REG_SP;
			tile->ins.index = REG_SP;
			tile->ins.base = REG_SP;
			tile->ins.disp = 8 * ref->id;
		}
		break;
	default:
		tile->ins.mod = MOD_REG;
		tile->operands[tile->arity++] = (struct operand){
			cover(expr), SLOT_RM | (is_dest ? SLOT_IS_DEST : 0) };
	}
}

static struct tile *
cover_varref(struct ast_varref *ref)
{
	struct tile *tile = calloc(1, sizeof *tile);
	tile->ins.opcode = OPC_MOV_RM();
	tile->ins.mod = MOD_MEM_LD;
	tile->ins.rm  = REG_SP;
	tile->ins.index = REG_SP;
	tile->ins.base = REG_SP;
	tile->ins.disp = 8 * ref->id;
	tile->genesis = SLOT_REG;
	return tile;
}

static struct tile *
cover_unop(struct ast_unop *unop)
{
	struct tile *tile;
	switch (unop->op) {
	case '~':
		tile = calloc(1, sizeof *tile + 1 * sizeof(struct operand));
		tile->ins.opcode = OPC_MUL_M();
		tile->ins.reg    = OPNO_NOT;
		tile->ins.mod    = MOD_REG;
		tile->operands[tile->arity++] = (struct operand){
			cover(unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;
	case '-':
		tile = calloc(1, sizeof *tile + 1 * sizeof(struct operand));
		tile->ins.opcode = OPC_MUL_M();
		tile->ins.reg    = OPNO_NEG;
		tile->ins.mod    = MOD_REG;
		tile->operands[tile->arity++] = (struct operand){
			cover(unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;
	default: return NULL;
	}
}

static struct tile *
cover_binop(struct ast_binop *binop)
{
	struct tile *tile;
	int num;
	switch (binop->op) {
	case '+': case '-': case '&': case '|': case '^':
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct operand));
		switch (binop->op) {
		case '+': num = OPNO_ADD; break;
		case '-': num = OPNO_SUB; break;
		case '&': num = OPNO_AND; break;
		case '|': num = OPNO_OR;  break;
		case '^': num = OPNO_XOR; break;
		}
		if (cover_immed(binop->rhs, tile)) {
			bool small_immed = tile->ins.immed >= SCHAR_MIN && tile->ins.immed <= SCHAR_MAX;
			tile->ins.opcode = OPC_ARITH_MI(small_immed);
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				cover(binop->lhs), SLOT_RM | SLOT_IS_DEST };
		} else {
			tile->ins.opcode = OPC_ARITH_RM(num);
			tile->operands[tile->arity++] = (struct operand){
				cover(binop->lhs), SLOT_REG | SLOT_IS_DEST };
			cover_modrm(binop->rhs, false, tile);
		}
		return tile;

	case LT2: case GT2:
		tile = calloc(1, sizeof *tile + 2 * sizeof(struct operand));
		switch (binop->op) {
		case LT2: num = OPNO_SHL; break;
		case GT2: num = OPNO_SAR; break;
		}
		if (cover_immed(binop->rhs, tile)) {
			tile->ins.opcode = OPC_SHIFT_MI();
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				cover(binop->lhs), SLOT_RM | SLOT_IS_DEST };
		} else {
			tile->ins.opcode = OPC_SHIFT_MC();
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				cover(binop->lhs), SLOT_RM | SLOT_IS_DEST };
			tile->operands[tile->arity++] = (struct operand){
				cover(binop->rhs), SLOT_NIL };
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

