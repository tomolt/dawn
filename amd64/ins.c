#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "../util.h"
#include "ins.h"

#define MAX_REGISTERS 2

#define SETREG(ins,r) do { if ((r) > 7) (ins).prefixes |= PFX_REX_R; (ins).reg = (r) & 7; } while (0)
#define SETRM(ins,r)  do { if ((r) > 7) (ins).prefixes |= PFX_REX_B; (ins).rm  = (r) & 7; } while (0)

static void
emit_prefixes(uint16_t prefixes, uint8_t **O)
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
		emit_immed(ins->disp, INS_DISP_WIDTH(ins), O);
	}
	if (INS_HAS_IMMED(ins)) {
		emit_immed(ins->immed, INS_IMMED_WIDTH(ins), O);
	}
}

static void
calc_reg_usage(struct tile *tile)
{
	// TODO change algorithm when switching to Sethi-Ullman allocation
	tile->holdregs = 1;
	tile->spill = 0;
	if (!tile->arity) {
		tile->maxregs++;
	}
	for (int i = 0; i < tile->arity; i++) {
		struct tile *oper = tile->operands[i];
		calc_reg_usage(oper);
		if (oper->maxregs + i > tile->maxregs) {
			tile->maxregs = oper->maxregs + i;
		}
	}
	if (tile->maxregs > MAX_REGISTERS) {
		tile->spill = tile->maxregs - MAX_REGISTERS;
		tile->maxregs = MAX_REGISTERS;
	}
}

static int
alloc_register(unsigned *registers)
{
	unsigned mask = 1u;
	for (int i = 0; i < MAX_REGISTERS; i++, mask<<=1) {
		if (!(*registers & mask)) {
			*registers |= mask;
			return i;
		}
	}
	assert(0);
}

static int
emit_rec(struct tile *tile, unsigned registers, void *stream)
{
	struct ins ins = { 0 };
	char regs[16];

	for (int i = 0; i < tile->arity; i++) {
		struct tile *oper = tile->operands[i];
		regs[i] = emit_rec(oper, registers, stream);
		if (i < tile->spill) {
			
			struct ins ins = { 0 };
			ins.opcode = 0x50 + (regs[i] & 7);
			if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
			uint8_t buf[32], *ptr = buf;
			emit_ins(&ins, &ptr);
			fwrite(buf, ptr - buf, 1, stream);

		} else {
			registers |= 1u << regs[i];
		}
	}
	if (!tile->arity) {
		regs[0] = alloc_register(&registers);
	}

	for (int i = tile->spill; i--;) {

		regs[i] = alloc_register(&registers);

		struct ins ins = { 0 };
		ins.opcode = 0x58 + (regs[i] & 7);
		if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
		uint8_t buf[32], *ptr = buf;
		emit_ins(&ins, &ptr);
		fwrite(buf, ptr - buf, 1, stream);
	}

	switch (tile->opclass) {
	case OPCL_ARITH_RM:
		ins.opcode = 0x08 * tile->opnum + 3;
		ins.has_modrm = true;
		ins.mod = MOD_REG;
		SETREG(ins, regs[0]);
		SETRM(ins, regs[1]);
		break;

	case OPCL_ARITH_MI:
		if (tile->immed <= SCHAR_MAX && tile->immed >= SCHAR_MIN) {
			ins.small_immed = true;
		}
		ins.opcode = ins.small_immed ? 0x83 : 0x81;
		ins.has_modrm = true;
		ins.has_immed = true;
		ins.mod = MOD_REG;
		ins.reg = tile->opnum;
		SETRM(ins, regs[0]);
		ins.immed = tile->immed;
		break;

	case OPCL_SHIFT_MC:
		ins.opcode = 0xD3;
		ins.has_modrm = true;
		ins.mod = MOD_REG;
		ins.reg = tile->opnum;
		SETRM(ins, regs[0]);
		break;

	case OPCL_SHIFT_MI:
		ins.opcode = 0xC1;
		ins.has_modrm = true;
		ins.has_immed = true;
		ins.small_immed = true;
		ins.mod = MOD_REG;
		ins.reg = tile->opnum;
		SETRM(ins, regs[0]);
		ins.immed = tile->immed;
		break;

	case OPCL_MOV_EI:
		ins.opcode = 0xB8 + (regs[0] & 7);
		if (regs[0] > 7) ins.prefixes |= PFX_REX_B;
		ins.has_immed = true;
		ins.immed = tile->immed;
		break;

	case OPCL_MOV_RM:
		ins.opcode = 0x8B;
		ins.has_modrm = true;
		ins.mod = MOD_MEM_SD;
		SETREG(ins, regs[0]);
		// HACK we hardcode stack behaviour here
		ins.rm    = REG_SP;
		ins.scale = 0;
		ins.index = REG_SP;
		ins.base  = REG_SP;
		ins.disp  = tile->immed;
		ins.prefixes |= PFX_REX_W;
		break;
	}

	uint8_t buf[32], *ptr = buf;
	emit_ins(&ins, &ptr);
	fwrite(buf, ptr - buf, 1, stream);

	return regs[0];
}

void
emit(struct tile *tile, void *stream)
{
	calc_reg_usage(tile);
	emit_rec(tile, 0, stream);
}

