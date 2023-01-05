
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
#include <assert.h>

#include "../pool.h"

#define NO_VAR SIZE_MAX
#define NUM_REGISTERS 16

enum {
	LOC_NONEXISTENT = 0,
	LOC_REGISTER,
	LOC_STACK
};

struct loc {
	int    kind;
	size_t address;
};

struct reg {
	size_t var;
	bool   available;
};

struct constraint {
	size_t var;
	struct loc loc;
};

struct tile {
	struct tile *next;
	int num_defs;
	int num_uses;
	struct constraint constraints[];
};

struct compiler {
	struct reg regs[NUM_REGISTERS];
	struct loc *var_locs;
	size_t *next_use;
	POOL tile_pool;
	struct tile *tile_stack;
};

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
spill(struct compiler *ctx, size_t var)
{
	(void)ctx, (void)var;
	struct loc loc;
	loc.kind = LOC_STACK;
	loc.address = 0;
	return loc;
}

static void
transfer(struct compiler *ctx, struct loc src, struct loc dest)
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
}

static void
accommodate(struct compiler *ctx, struct constraint *con)
{
	if (ctx->var_locs[con->var].kind == LOC_REGISTER) {
		 con->loc = ctx->var_locs[con->var];
		 return;
	}

	int reg = allocate(ctx);
	con->loc.kind = LOC_REGISTER;
	con->loc.address = reg;

	if (!ctx->regs[reg].available) {
		size_t var = ctx->regs[reg].var;
		ctx->var_locs[var] = spill(var);
		transfer(ctx, ctx->var_locs[var], loc);
		ctx->regs[reg].available = true;
	}
}

void
compile(const struct museq *seq)
{
	struct compiler compiler = { 0 }, *ctx = &compiler;
	ctx->var_locs = calloc(seq->count, sizeof *ctx->var_locs);
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

		cover(ctx, seq, cursor);

		for (constraints) {
			con.loc = accommodate(con);
		}

		for (uses) {
			transfer(ctx->var_locs[con.var], con.loc);
			ctx->var_locs[con.var] = con.loc;
		}

		add ins to stack;

		for (defs) {
			transfer(con.loc, var_locs[]);
			release(var_locs[]);
		}

		for () {
			emit_ins();
		}

		ctx->tile_stack = NULL;
		pool_release(&ctx->tile_pool);
	}

	free(ctx->var_locs);
	free(ctx->next_use);
}
