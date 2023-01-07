#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "muop.h"

int
muop_arity(uint8_t op)
{
	switch (op) {
	case MU_IMM: return 0;
	case MU_COPY: case MU_NEG: case MU_NOT: case MU_LDL: case MU_STL: case MU_LDR:
		     return 1;
	default: return 2;
	}
}

bool
muop_side_effects(uint8_t op)
{
	return op == MU_STL || op == MU_STR;
}

bool
muop_valued(uint8_t op)
{
	return op != MU_STL && op != MU_STR;
}

bool
muop_commutative(uint8_t op)
{
	switch (op) {
	case MU_ADD: case MU_AND: case MU_OR: case MU_XOR: case MU_MUL:
		return true;
	default: return false;
	}
}

size_t
museq_append(struct museq *museq, uint8_t op, size_t arg1, size_t arg2)
{
	if (museq->count == museq->capac) {
		museq->capac *= 2;
		if (!museq->capac) museq->capac = 16;
		museq->muops = realloc(museq->muops, museq->capac * sizeof *museq->muops);
	}
	size_t idx = museq->count++;

	museq->muops[idx].op = op;
	museq->muops[idx].arg1 = (uint16_t)(idx - arg1);
	museq->muops[idx].arg2 = (uint16_t)(idx - arg2);

	return idx;
}

size_t
museq_append_imm(struct museq *museq, int32_t value)
{
	if (museq->count == museq->capac) {
		museq->capac *= 2;
		if (!museq->capac) museq->capac = 16;
		museq->muops = realloc(museq->muops, museq->capac * sizeof *museq->muops);
	}
	size_t idx = museq->count++;

	museq->muops[idx].op = MU_IMM;
	museq->muops[idx].arg1 = (uint16_t)value;
	museq->muops[idx].arg2 = (uint16_t)(value >> 16);

	return idx;
}

static const char *muop_names[256] = {
	[MU_COPY] = "copy",
	[MU_NEG] = "neg",
	[MU_NOT] = "not",
	[MU_ADD] = "add",
	[MU_SUB] = "sub",
	[MU_AND] = "and",
	[MU_OR ] = "or",
	[MU_XOR] = "xor",
	[MU_MUL] = "mul",
	[MU_DIV] = "div",
	[MU_MOD] = "mod",
	[MU_SHL] = "shl",
	[MU_SHR] = "shr",
	[MU_LDL] = "ldl",
	[MU_STL] = "stl",
	[MU_LDR] = "ldr",
	[MU_STR] = "str",
	[MU_IMM] = "imm",
};

void
museq_format(const struct museq *museq, void *file)
{
	for (size_t i = 0; i < museq->count; i++) {
		const struct muop *muop = &museq->muops[i];
		fprintf(file, "%4zu:  %s\t", i, muop_names[muop->op]);
		switch (muop->op) {
		case MU_IMM:
			fprintf(file, "$%" PRId32 "\n", (int32_t)muop->arg1 |
				((int32_t)muop->arg2 << 16));
			break;

		case MU_COPY: case MU_NEG: case MU_NOT: case MU_LDL: case MU_LDR:
			fprintf(file, "%%%zu\n",
				i - muop->arg1);
			break;

		default:
			fprintf(file, "%%%zu,\t%%%zu\n",
				i - muop->arg1, i - muop->arg2);
			break;
		}
	}
}
