#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "../syntax.h"
#include "../ast.h"
#include "../pool.h"
#include "ins.h"

#define NEW_TILE(ctx, maxops) pool_alloc(&(ctx)->tile_pool,		\
	sizeof(struct tile) + (maxops) * sizeof(struct operand))

typedef struct tiler T;

struct tile *tile_from_expr(T *ctx, EXPR expr);

static struct tile *
tile_from_literal(T *ctx, struct ast_literal *literal)
{
	struct tile *tile = NEW_TILE(ctx, 0);
	tile->ins.opcode = OPC_MOV_EI();
	tile->ins.immed = literal->value;
	tile->genesis = SLOT_EMB;
	return tile;
}

static bool
immed_from_expr(T *ctx, EXPR expr, struct tile *tile)
{
	(void)ctx;
	if (EXPR_KIND(expr) != EXPR_literal) {
		return false;
	}
	tile->ins.immed = ((struct ast_literal *)expr)->value;
	return true;
}

static void
modrm_from_expr(T *ctx, EXPR expr, bool is_dest, struct tile *tile)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_varref:
		{
			const struct ast_varref *ref = (struct ast_varref *)expr;
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
			tile_from_expr(ctx, expr), SLOT_RM | (is_dest ? SLOT_IS_DEST : 0) };
	}
}

static struct tile *
tile_from_varref(T *ctx, struct ast_varref *ref)
{
	struct tile *tile = NEW_TILE(ctx, 0);
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
tile_from_unop(T *ctx, struct ast_unop *unop)
{
	struct tile *tile;
	switch (unop->op) {
	case '~':
		tile = NEW_TILE(ctx, 1);
		tile->ins.opcode = OPC_MUL_M();
		tile->ins.reg    = OPNO_NOT;
		tile->ins.mod    = MOD_REG;
		tile->operands[tile->arity++] = (struct operand){
			tile_from_expr(ctx, unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;
	case '-':
		tile = NEW_TILE(ctx, 1);
		tile->ins.opcode = OPC_MUL_M();
		tile->ins.reg    = OPNO_NEG;
		tile->ins.mod    = MOD_REG;
		tile->operands[tile->arity++] = (struct operand){
			tile_from_expr(ctx, unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;
	default: assert(0); return NULL;
	}
}

static struct tile *
tile_from_binop(T *ctx, struct ast_binop *binop)
{
	struct tile *tile;
	int num;
	switch (binop->op) {
	case '+': case '-': case '&': case '|': case '^':
		tile = NEW_TILE(ctx, 2);
		switch (binop->op) {
		case '+': num = OPNO_ADD; break;
		case '-': num = OPNO_SUB; break;
		case '&': num = OPNO_AND; break;
		case '|': num = OPNO_OR;  break;
		case '^': num = OPNO_XOR; break;
		}
		if (immed_from_expr(ctx, binop->rhs, tile)) {
			bool small_immed = tile->ins.immed >= SCHAR_MIN && tile->ins.immed <= SCHAR_MAX;
			tile->ins.opcode = OPC_ARITH_MI(small_immed);
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				tile_from_expr(ctx, binop->lhs), SLOT_RM | SLOT_IS_DEST };
		} else {
			tile->ins.opcode = OPC_ARITH_RM(num);
			tile->operands[tile->arity++] = (struct operand){
				tile_from_expr(ctx, binop->lhs), SLOT_REG | SLOT_IS_DEST };
			modrm_from_expr(ctx, binop->rhs, false, tile);
		}
		return tile;

	case LT2: case GT2:
		tile = NEW_TILE(ctx, 2);
		switch (binop->op) {
		case LT2: num = OPNO_SHL; break;
		case GT2: num = OPNO_SAR; break;
		}
		if (immed_from_expr(ctx, binop->rhs, tile)) {
			tile->ins.opcode = OPC_SHIFT_MI();
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				tile_from_expr(ctx, binop->lhs), SLOT_RM | SLOT_IS_DEST };
		} else {
			tile->ins.opcode = OPC_SHIFT_MC();
			tile->ins.reg    = num;
			tile->ins.mod    = MOD_REG;
			tile->operands[tile->arity++] = (struct operand){
				tile_from_expr(ctx, binop->lhs), SLOT_RM | SLOT_IS_DEST };
			tile->operands[tile->arity++] = (struct operand){
				tile_from_expr(ctx, binop->rhs), SLOT_NIL };
		}
		return tile;

	default: assert(0); return NULL;
	}
}

struct tile *
tile_from_expr(T *ctx, EXPR expr)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_literal: return tile_from_literal(ctx, (struct ast_literal *)expr);
	case EXPR_varref:  return tile_from_varref (ctx, (struct ast_varref  *)expr);
	case EXPR_unop:    return tile_from_unop   (ctx, (struct ast_unop    *)expr);
	case EXPR_binop:   return tile_from_binop  (ctx, (struct ast_binop   *)expr);
	default: assert(0); return NULL;
	}
}

struct tile *
dawn_cover_ast(T *ctx, STMT stmt)
{
	switch (STMT_KIND(stmt)) {
	case STMT_exprstmt: return tile_from_expr(ctx, ((struct ast_exprstmt *)stmt)->expr);
	default: assert(0); return NULL;
	}
}

