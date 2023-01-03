#include <stdint.h>
#include <stdlib.h>

#include "ins.h"

#define MAX_REGISTERS 16

#define VACANT UINT32_MAX

typedef uint32_t vreg_num;

#define CON_NONE 0x00
#define CON_HARD 0x40
#define CON_SOFT 0x80
#define CON_TYPE(c)     ((c)&0xC0)
#define CON_REGISTER(c) ((c)&0x3F)

struct live_range {
	size_t   begin;
	size_t   end;
	vreg_num vreg;
	uint8_t  constraint;
};

struct allocator {
	int num_active;
	struct live_range active[MAX_REGISTERS];
	vreg_num utilization[MAX_REGISTERS];
	int8_t *assignments;
};

activate_range()
{
}

static void
retire_ranges(struct allocator *ctx, cutoff)
{
	while (ctx->num_active) {
		struct live_range *range = &ctx->active[ctx->num_active-1];
		if (range->end > cutoff) break;
		int reg = ctx->assignments[range->vreg];
		if (!(reg < 0)) ctx->utilization[reg] = VACANT;
		ctx->num_active--;
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
		if (ctx->utilization[reg] == VACANT) {
			return reg;
		}
	}

	for (reg = 0; reg < MAX_REGISTERS; reg++) {
		if (ctx->utilization[reg] == VACANT) {
			return reg;
		}
	}

	return -1;
}

static int
evict_register()
{
	int reg;

	for (reg = 0; reg < MAX_REGISTERS; reg++) {
	}

	return -1;
}

static int
compare_range_begin(const void *a, const void *b)
{
	const struct live_range *ar = a, *br = b;
	return ar->begin - br->begin;
}

void
dawn_regalloc(size_t num_ranges, struct live_range *ranges,
	vreg_num num_vregs, int8_t *assignments)
{
	qsort(ranges, num_ranges, sizeof(struct live_range), compare_range_begin);

	for (size_t cursor = 0; cursor < num_ranges; cursor++) {
		range = ranges[cursor];

		retire_ranges();
		insert_sorted(active, range);

		int reg = select_register();
		if (reg < 0) {
			reg = evict_register();
		}
		if (utilization[reg] != VACANT) {
			assignments[utilization[reg]] = -1;
		}

		utilization[reg] = range.vreg;
		assignments[range.vreg] = reg;
	}

	free(ranges);
}
