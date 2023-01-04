#include <stdint.h>
#include <stdlib.h>

#include "ins.h"

static void
assemble_prefixes(uint16_t prefixes, uint8_t **O)
{
	if (prefixes & PFX_FS) *(*O)++ = 0x64;
	if (prefixes & PFX_GS) *(*O)++ = 0x65;
	if (prefixes & PFX_DATASZ) *(*O)++ = 0x66;
	if (prefixes & PFX_ADDRSZ) *(*O)++ = 0x67;

	if (prefixes & PFX_REX_MASK) {
		uint8_t rex = 0x40;
		if (prefixes & PFX_REX_W) rex |= 0x08;
		if (prefixes & PFX_REX_R) rex |= 0x04;
		if (prefixes & PFX_REX_X) rex |= 0x02;
		if (prefixes & PFX_REX_B) rex |= 0x01;
		*(*O)++ = rex;
	}
	
	if (prefixes & PFX_0F) *(*O)++ = 0x0F;
}

static void
assemble_immed(int64_t immed, int width, uint8_t **O)
{
	for (int i = 0; i < 1 << width; i++) {
		*(*O)++ = immed;
		immed >>= 8;
	}
}

void
dawn_assemble_ins(const struct ins *ins, uint8_t **O)
{
	if (ins->prefixes) assemble_prefixes(ins->prefixes, O);
	*(*O)++ = ins->opcode & 0xFF;
	if (INS_HAS_MODRM(ins)) {
		*(*O)++ = (ins->mod<<6) | (ins->reg<<3) | ins->rm;
	}
	if (INS_HAS_SIB(ins)) {
		*(*O)++ = (ins->scale<<6) | (ins->index<<3) | ins->base;
	}
	if (INS_HAS_DISP(ins)) {
		assemble_immed(ins->disp, INS_DISP_WIDTH(ins), O);
	}
	if (INS_HAS_IMMED(ins)) {
		assemble_immed(ins->immed, INS_IMMED_WIDTH(ins), O);
	}
}

size_t
iseq_append(struct iseq *iseq, struct ins *ins)
{
	if (iseq->count == iseq->capac) {
		iseq->capac *= 2;
		if (!iseq->capac) iseq->capac = 16;
		iseq->ins = realloc(iseq->ins, iseq->capac * sizeof *ins);
	}
	size_t idx = iseq->count++;
	iseq->ins[idx] = *ins;
	return idx;
}
