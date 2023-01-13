
/*
 * - Near-perfect register allocation for linear blocks
 * - lifetime splitting / second-chance allocation
 * - x86 instruction selection with operand fusion
 * - dead-code elimination
 * - spill-code insertion and fusion
 * - assembling
 * - rematerialization
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "compile.h"
#include "muop.h"
#include "risc-v/ins.h"
#include "revbuf.h"

extern const struct template *riscv_tile(const struct museq *museq, size_t index, size_t *bindings);
extern void riscv_assemble(const struct template *template, const size_t *bindings, struct revbuf *revbuf);

void
compile(const struct museq *museq, void *file)
{
	struct revbuf revbuf = { 0 };
	size_t bindings[32];
	for (size_t index = museq->count; index--;) {
		const struct template *template = riscv_tile(museq, index, bindings);
		for (int i = 0; template->constraints[i]; i++) {
			if (template->constraints[i] != IMM) {
				bindings[i] += 5;
			}
		}
		riscv_assemble(template, bindings, &revbuf);
	}
	fwrite(revbuf.data + revbuf.start, 1, revbuf.capac - revbuf.start, file);
	free(revbuf.data);
}

#if 0
calc_usecounts()
{
	for (size_t i = museq->count; i--;) {
		if (has_side_effects(op)) {
			usecounts[i]++;
		}
		switch (num_dependencies(op)) {
		case 2:
			usecounts[i - op->arg2]++;
			/* fallthrough */
		case 1:
			usecounts[i - op->arg1]++;
		}
	}
}

drop_deps()
{
	switch (num_dependencies(op)) {
	case 2:
		usecounts[i - op->arg2]--;
		/* fallthrough */
	case 1:
		usecounts[i - op->arg1]--;
	}
}
#endif

#if 0
static int
allocate(struct compiler *ctx)
{
	for (int i = 0; i < NUM_REGISTERS; i++) {
		if (ctx->regs[i].available) return i;
	}

	int candidate = 0;
	size_t furthest_use = ctx->next_use[ctx->regs[0].var];
	for (int i = 1; i < NUM_REGISTERS; i++) {
		size_t next_use = ctx->next_use[ctx->regs[i].var];
		if (next_use > furthest_use) {
			candidate = i;
			furthest_use = next_use;
		}
	}

	return candidate;
}

static struct loc
assign_spill_loc(struct compiler *ctx, size_t var)
{
	(void)ctx, (void)var;
	struct loc loc;
	loc.kind = LOC_STACK;
	loc.address = 0;
	return loc;
}

static void
transfer(struct compiler *ctx, size_t var, struct loc dest)
{
	if (src.kind  == LOC_NONEXISTENT) return;
	if (dest.kind == LOC_NONEXISTENT) return;
	if (src.kind == dest.kind && src.address == dest.address) return;

	if (src.kind == LOC_REGISTER && dest.kind == LOC_REGISTER) {
		X86_MOV_RR;
	} else if (src.kind == LOC_REGISTER && dest.kind == LOC_STACK) {
		X86_MOV_MR;
	} else if (src.kind == LOC_STACK && dest.kind == LOC_REGISTER) {
		X86_MOV_RM;
	} else {
		assert(0);
	}

	if (var.loc.kind == LOC_REGISTER) {
		int reg = var.loc.address;
		ctx->regs[reg].available = true;
	}
	var.loc = dest;
	if (var.loc.kind == LOC_REGISTER) {
		int reg = var.loc.address;
		ctx->regs[reg].available = false;
		ctx->regs[reg].var = var;
	}
}

gen_spill()
{
}

gen_unspill()
{
}

static void
satisfy(struct compiler *ctx, struct constraint *con, struct tile **tile_stack)
{
	if (ctx->var_loc[con->var].kind == LOC_REGISTER) {
		 con->loc = ctx->var_loc[con->var];
		 return;
	}

	int reg = allocate(ctx);

	if (!ctx->regs[reg].available) {
		size_t var = ctx->regs[reg].var;
		ctx->var_loc[var] = spill(var);
		transfer(ctx, ctx->var_loc[var], loc, tile_stack);
		ctx->regs[reg].available = true;
	}

	con->loc.kind = LOC_REGISTER;
	con->loc.address = reg;
	transfer(ctx, ctx->var_loc[var], loc, tile_stack);
}

void
compile(const struct museq *seq)
{
	struct compiler compiler = { 0 }, *ctx = &compiler;
	ctx->var_loc = calloc(seq->count, sizeof *ctx->var_loc);
	ctx->next_use = calloc(seq->count, sizeof *ctx->next_use);
	for (size_t i = 0; i < seq->count; i++) {
		ctx->next_use[i] = SIZE_MAX;
	}
	for (int i = 0; i < NUM_REGISTERS; i++) {
		ctx->regs[i].available = true;
	}

	for (size_t cursor = seq->count; cursor--;) {
#if 0
		if (!usecounts[cursor]) {
			drop_deps();
			continue;
		}
#endif

		tile(ctx, seq, cursor);

		for (defs) {
			satisfy(ctx, con);
			if (def.var.spill.kind != LOC_NONEXISTENT) {
				gen_spill();
			}
		}
		for (defs) {
			def.var.loc.address
			release();
		}

		for (uses) {
			satisfy(ctx, con);
			ctx->var_loc[con->var] = con->loc;
		}

		emit(tile);
	}

	free(ctx->vars);
}
#endif
