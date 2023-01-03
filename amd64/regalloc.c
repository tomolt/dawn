#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#include "ins.h"

struct allocator {
	unsigned available;
	struct live_range active[MAX_REGISTERS];
	int prev[MAX_REGISTERS+1];
	int next[MAX_REGISTERS+1];
};

#define FIRST(ctx) ((ctx)->next[MAX_REGISTERS])
#define LAST(ctx)  ((ctx)->prev[MAX_REGISTERS])

static void
occupy_register(struct allocator *ctx, int reg, struct live_range range)
{
	ctx->available &= ~(1u - reg);
	ctx->active[reg] = range;

	int prev = MAX_REGISTERS;
	int next = FIRST(ctx);
	while (next < MAX_REGISTERS && ctx->active[next].end < range.end) {
		prev = next;
		next = ctx->next[next];
	}

	ctx->next[reg] = next;
	ctx->prev[reg] = prev;
	ctx->next[prev] = reg;
	ctx->prev[next] = reg;
}

static void
clear_register(struct allocator *ctx, int reg)
{
	ctx->available |= 1u << reg;

	int prev = ctx->prev[reg];
	int next = ctx->next[reg];
	ctx->next[prev] = next;
	ctx->prev[next] = prev;
}

static void
clear_expired(struct allocator *ctx, size_t cutoff)
{
	int reg = FIRST(ctx);
	while (reg < MAX_REGISTERS && ctx->active[reg].end <= cutoff) {
		int next = ctx->next[reg];
		clear_register(ctx, reg);
		reg = next;
	}
}

static int
select_register(struct allocator *ctx, uint8_t constraint)
{
	int reg;

	switch (CON_TYPE(constraint)) {
	case CON_HARD:
		return CON_REGISTER(constraint);

	case CON_SOFT:
		reg = CON_REGISTER(constraint);
		if ((ctx->available >> reg) & 1) return reg;
	}

	return ffs(ctx->available) - 1;
}

static int
evict_register(struct allocator *ctx, struct live_range range)
{
	int last = LAST(ctx);
	struct live_range *last_range = &ctx->active[last];
	if (last_range->end > range.end) return last;
	if (last_range->begin < range.begin) return last;
	return -1;
}

static int
compare_range_begin(const void *a, const void *b)
{
	const struct live_range *ar = a, *br = b;
	return ar->begin - br->begin;
}

void
dawn_allocate_registers(size_t num_ranges, struct live_range *ranges,
	int8_t *assignment)
{
	struct allocator ctx = { 0 };
	ctx.available = (MAX_REGISTERS - 1) & ~(1u << REG_SP);
	ctx.prev[MAX_REGISTERS] = MAX_REGISTERS;
	ctx.next[MAX_REGISTERS] = MAX_REGISTERS;

	qsort(ranges, num_ranges, sizeof(struct live_range), compare_range_begin);

	for (size_t cursor = 0; cursor < num_ranges; cursor++) {
		struct live_range range = ranges[cursor];

		clear_expired(&ctx, range.begin);

		int reg = select_register(&ctx, range.constraint);
		if (reg < 0) {
			reg = evict_register(&ctx, range);
			if (reg < 0) {
				assignment[range.vreg] = -1;
				continue;
			}
		}

		if (!((ctx.available >> reg) & 1)) {
			clear_register(&ctx, reg);
		}
		
		occupy_register(&ctx, reg, range);
		assignment[range.vreg] = reg;
	}

	free(ranges);
}
