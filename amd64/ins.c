#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "../util.h"
#include "ins.h"

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
grab_register(unsigned *regs)
{
	int preference[] = { 3, 6, 7, 0, 1, 2, 8, 9, 10, 11, 12, 13, 14, 15 };
	for (int i = 0; i < (int)(sizeof(preference) / sizeof(*preference)); i++) {
		unsigned mask = 1 << preference[i];
		if (*regs & mask) {
			*regs &= ~mask;
			return preference[i];
		}
	}
	return -1;
}

static void
emit_rec(struct tile *tile, int dest, unsigned availregs, void *stream)
{
	struct ins ins = { 0 };
	char regs[16];
	memset(regs, -1, sizeof regs);
	regs[0] = dest;

	switch (tile->opclass) {
	case OPCL_SHIFT_MC:
		availregs &= ~(1u << REG_CX);
		regs[1] = REG_CX;
		break;
	}

	for (int i = 0; i < tile->arity; i++) {
		if (regs[i] < 0) {
			regs[i] = grab_register(&availregs);
		}
		emit_rec(tile->operands[i], regs[i], availregs, stream);
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
	}

	uint8_t buf[32], *ptr = buf;
	emit_ins(&ins, &ptr);
	fwrite(buf, ptr - buf, 1, stream);
}

void
emit(struct tile *tile, void *stream)
{
	emit_rec(tile, 0, ~((1u << REG_SP) | (1u << REG_AX)), stream);
}

