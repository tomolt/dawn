#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "../syntax.h"
#include "../ast.h"
#include "../pool.h"
#include "ins.h"

typedef struct tiler T;

static vreg_num tile_expr(T *ctx, EXPR expr);

static void
append_patch(T *ctx, size_t ins_idx, uint8_t slot, vreg_num vreg)
{
	if (ctx->num_patches == ctx->capac_patches) {
		ctx->capac_patches *= 2;
		if (!ctx->capac_patches) ctx->capac_patches = 16;
		ctx->patches = realloc(ctx->patches,
			ctx->capac_patches * sizeof *ctx->patches);
	}
	size_t idx = ctx->num_patches++;
	ctx->patches[idx].ins_idx = ins_idx;
	ctx->patches[idx].slot    = slot;
	ctx->patches[idx].vreg    = vreg;
}

static vreg_num
tile_literal(T *ctx, struct ast_literal *literal)
{
	struct ins ins = { 0 };
	ins.opcode = OPC_MOV_EI();
	ins.immed = literal->value;
	size_t ins_idx = iseq_append(&ctx->iseq);
	vreg_num dest_vreg = ctx->num_vregs++;
	append_patch(ctx, ins_idx, SLOT_EMB, dest_vreg);
	return dest_vreg;
}

static bool
immed_from_expr(T *ctx, EXPR expr, struct ins *ins)
{
	(void)ctx;
	if (EXPR_KIND(expr) != EXPR_literal) {
		return false;
	}
	ins->immed = ((struct ast_literal *)expr)->value;
	return true;
}

static void
modrm_from_expr(T *ctx, EXPR expr, struct ins *ins)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_varref:
		{
			const struct ast_varref *ref = (struct ast_varref *)expr;
			ins.mod   = MOD_MEM_LD;
			ins.rm    = REG_SP;
			ins.index = REG_SP;
			ins.base  = REG_SP;
			ins.disp  = 8 * ref->id;
		}
		break;
	default:
		ins.mod = MOD_REG;
		append_patch(ctx, );
		operands[arity++] = (struct operand){
			tile_expr(ctx, expr), SLOT_RM | (is_dest ? SLOT_IS_DEST : 0) };
	}
}

static vreg_num
tile_varref(T *ctx, struct ast_varref *ref)
{
	struct ins ins = { 0 };
	ins.opcode = OPC_MOV_RM();
	ins.mod    = MOD_MEM_LD;
	ins.rm     = REG_SP;
	ins.index  = REG_SP;
	ins.base   = REG_SP;
	ins.disp   = 8 * ref->id;
}

static vreg_num
tile_unop(T *ctx, struct ast_unop *unop)
{
	struct ins ins = { 0 };
	switch (unop->op) {
	case '~':
		ins.opcode = OPC_MUL_M();
		ins.reg    = OPNO_NOT;
		ins.mod    = MOD_REG;
		operands[tile->arity++] = (struct operand){
			tile_expr(ctx, unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;

	case '-':
		ins.opcode = OPC_MUL_M();
		ins.reg    = OPNO_NEG;
		ins.mod    = MOD_REG;
		operands[tile->arity++] = (struct operand){
			tile_expr(ctx, unop->arg), SLOT_RM | SLOT_IS_DEST };
		return tile;

	default: assert(0); return NULL;
	}
}

static vreg_num
tile_binop(T *ctx, struct ast_binop *binop)
{
	struct ins ins = { 0 };
	size_t ins_idx;
	vreg_num lhs_vreg;
	int num;

	switch (binop->op) {
	case '+': case '-': case '&': case '|': case '^':
		switch (binop->op) {
		case '+': num = OPNO_ADD; break;
		case '-': num = OPNO_SUB; break;
		case '&': num = OPNO_AND; break;
		case '|': num = OPNO_OR;  break;
		case '^': num = OPNO_XOR; break;
		}
		lhs_vreg = tile_expr(ctx, binop->lhs);
		if (immed_from_expr(ctx, binop->rhs, &ins)) {
			bool small_immed = ins.immed >= SCHAR_MIN && ins.immed <= SCHAR_MAX;
			ins.opcode = OPC_ARITH_MI(small_immed);
			ins.reg    = num;
			ins.mod    = MOD_REG;
		} else {
			ins.opcode = OPC_ARITH_RM(num);
			operands[tile->arity++] = (struct operand){
				tile_expr(ctx, binop->lhs), SLOT_REG | SLOT_IS_DEST };
			ins_idx = iseq_append(&ctx->iseq);
			append_patch(ctx, ins_idx, SLOT_MODRM, tile_expr(ctx, binop->lhs));
			modrm_from_expr(ctx, binop->rhs, false, tile);
		}
		ins_idx = iseq_append(&ctx->iseq);
		append_patch(ctx, ins_idx, SLOT_MODRM, lhs_vreg);
		return;

	case LT2: case GT2:
		switch (binop->op) {
		case LT2: num = OPNO_SHL; break;
		case GT2: num = OPNO_SAR; break;
		}
		if (immed_from_expr(ctx, binop->rhs, &ins)) {
			ins.opcode = OPC_SHIFT_MI();
			ins.reg    = num;
			ins.mod    = MOD_REG;
			ins_idx = iseq_append(&ctx->iseq);
			append_patch(ins_idx, SLOT_MODRM, tile_expr(ctx, binop->lhs));
		} else {
			ins.opcode = OPC_SHIFT_MC();
			ins.reg    = num;
			ins.mod    = MOD_REG;
			operands[tile->arity++] = (struct operand){
				tile_expr(ctx, binop->lhs), SLOT_RM | SLOT_IS_DEST };
			operands[tile->arity++] = (struct operand){
				tile_expr(ctx, binop->rhs), SLOT_NIL };
		}
		return;

	default: assert(0); return NULL;
	}
}

static vreg_num
tile_expr(T *ctx, EXPR expr)
{
	switch (EXPR_KIND(expr)) {
	case EXPR_literal: return tile_literal(ctx, (struct ast_literal *)expr);
	case EXPR_varref:  return tile_varref (ctx, (struct ast_varref  *)expr);
	case EXPR_unop:    return tile_unop   (ctx, (struct ast_unop    *)expr);
	case EXPR_binop:   return tile_binop  (ctx, (struct ast_binop   *)expr);
	default: assert(0); return 0;
	}
}

void
dawn_cover_ast(T *ctx, STMT stmt)
{
	switch (STMT_KIND(stmt)) {
	case STMT_exprstmt: tile_expr(ctx, ((struct ast_exprstmt *)stmt)->expr); break;
	default: assert(0); break;
	}
}
