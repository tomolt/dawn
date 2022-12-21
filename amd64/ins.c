#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "../util.h"
#include "ins.h"

static const int avail_registers[] = {
	0, 1, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static void
emit_prefixes(uint16_t prefixes, uint8_t **O)
{
	if (prefixes & PFX_FS) *(*O)++ = 0x64;
	if (prefixes & PFX_GS) *(*O)++ = 0x65;
	if (prefixes & PFX_DATASZ) *(*O)++ = 0x66;
	if (prefixes & PFX_ADDRSZ) *(*O)++ = 0x67;

	if (prefixes & PFX_REX_W) *(*O)++ = 0x48;
	
	if (prefixes & PFX_0F) *(*O)++ = 0x0F;
}

static void
emit_immed(int64_t immed, int width, uint8_t **O)
{
	for (int i = 0; i < 1 << width; i++) {
		*(*O)++ = immed;
		immed >>= 8;
	}
}

static void
emit_ins(const struct ins *ins, uint8_t **O)
{
	if (ins->prefixes) emit_prefixes(ins->prefixes, O);
	*(*O)++ = ins->opcode;
	if (INS_HAS_MODRM(ins)) {
		*(*O)++ = (ins->mod<<6) | (ins->reg<<3) | ins->rm;
	}
	if (INS_HAS_SIB(ins)) {
		*(*O)++ = (ins->scale<<6) | (ins->index<<3) | ins->base;
	}
	if (INS_HAS_DISP(ins)) {
		emit_immed(ins->disp, 2, O);
	}
	if (INS_HAS_IMMED(ins)) {
		emit_immed(ins->immed, INS_IMMED_WIDTH(ins), O);
	}
}

static int
emit_rec(struct tile *tile, int nextreg, void *stream)
{
	struct ins ins = { 0 };
	int regs[16];

	for (int i = 0; i < tile->arity; i++) {
		regs[i] = emit_rec(tile->operands[i], nextreg + i, stream);
	}
	regs[tile->arity] = avail_registers[nextreg];

	switch (tile->opclass) {
	case OPCL_ARITH_RM:
		ins.opcode = 0x08 * tile->opnum + 3;
		ins.has_modrm = true;
		ins.mod = MOD_REG;
		ins.reg = regs[0];
		ins.rm  = regs[1];
		break;
	case OPCL_ARITH_RI:
		if (tile->immed <= SCHAR_MAX && tile->immed >= SCHAR_MIN) {
			ins.small_immed = true;
		}
		ins.opcode = ins.small_immed ? 0x83 : 0x81;
		ins.has_modrm = true;
		ins.has_immed = true;
		ins.mod = MOD_REG;
		ins.reg = tile->opnum;
		ins.rm  = regs[0];
		ins.immed = tile->immed;
		break;
	case OPCL_MOV_EI:
		ins.opcode = 0xB8 + regs[0];
		ins.has_immed = true;
		ins.immed = tile->immed;
		break;
	}

	uint8_t buf[32], *ptr = buf;
	emit_ins(&ins, &ptr);
	fwrite(buf, ptr - buf, 1, stream);

	return regs[tile->arity];
}

void
emit(struct tile *tile, void *stream)
{
	emit_rec(tile, 0, stream);
}

