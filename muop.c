#include <stdint.h>
#include <stdlib.h>

#include "muop.h"

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

