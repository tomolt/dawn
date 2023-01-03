#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "ins.h"

extern void dawn_assemble_ins(const struct ins *ins, uint8_t **O);

static void
sort_operands(struct tile *tile)
{
	/* insertion sort */
	for (int i = 1; i < tile->arity; i++) {
		// TODO consider holdregs
		for (int j = i; j > 0 && tile->operands[j].tile->maxregs > tile->operands[j-1].tile->maxregs; j--) {
			struct operand temp = tile->operands[j];
			tile->operands[j] = tile->operands[j-1];
			tile->operands[j-1] = temp;
		}
	}
}

static void
calc_reg_usage(struct tile *tile)
{
	tile->holdregs = 1;
	tile->spill = 0;
	if (!tile->arity) {
		tile->maxregs++;
	}
	for (int i = 0; i < tile->arity; i++) {
		struct operand *opd = &tile->operands[i];
		calc_reg_usage(opd->tile);
	}
	sort_operands(tile);
	for (int i = 0; i < tile->arity; i++) {
		struct operand *opd = &tile->operands[i];
		if (opd->tile->maxregs + i > tile->maxregs) {
			tile->maxregs = opd->tile->maxregs + i;
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
sethi_ullman_rec(struct tile *tile, unsigned registers, void *stream)
{
	struct ins ins = tile->ins;
	char regs[16];
	int dest = -1;

	for (int i = 0; i < tile->arity; i++) {
		const struct operand *opd = &tile->operands[i];
		regs[i] = sethi_ullman_rec(opd->tile, registers, stream);
		if (i < tile->spill) {
			
			struct ins ins = { 0 };
			ins.opcode = 0x50 + (regs[i] & 7);
			if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
			uint8_t buf[32], *ptr = buf;
			dawn_assemble_ins(&ins, &ptr);
			fwrite(buf, ptr - buf, 1, stream);

		} else {
			registers |= 1u << regs[i];
		}
	}
	if (tile->genesis != SLOT_NIL) {
		dest = alloc_register(&registers);
		switch (tile->genesis & 0xFF) {
		case SLOT_EMB:
			ins.opcode += dest & 7;
			if (dest > 7) ins.prefixes |= PFX_REX_B;
			break;
		case SLOT_REG:
			ins.reg = dest & 7;
			if (dest > 7) ins.prefixes |= PFX_REX_R;
			break;
		case SLOT_RM:
			ins.rm = dest & 7;
			if (dest > 7) ins.prefixes |= PFX_REX_B;
			break;
		}
	}

	for (int i = tile->spill; i--;) {

		regs[i] = alloc_register(&registers);

		struct ins ins = { 0 };
		ins.opcode = 0x58 + (regs[i] & 7);
		if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
		uint8_t buf[32], *ptr = buf;
		dawn_assemble_ins(&ins, &ptr);
		fwrite(buf, ptr - buf, 1, stream);
	}

	for (int i = 0; i < tile->arity; i++) {
		const struct operand *opd = &tile->operands[i];
		switch (opd->slot & 0xFF) {
		case SLOT_EMB:
			ins.opcode += regs[i] & 7;
			if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
			break;
		case SLOT_REG:
			ins.reg = regs[i] & 7;
			if (regs[i] > 7) ins.prefixes |= PFX_REX_R;
			break;
		case SLOT_RM:
			ins.rm = regs[i] & 7;
			if (regs[i] > 7) ins.prefixes |= PFX_REX_B;
			break;
		}
		if (opd->slot & SLOT_IS_DEST) {
			dest = regs[i];
		}
	}

	uint8_t buf[32], *ptr = buf;
	dawn_assemble_ins(&ins, &ptr);
	fwrite(buf, ptr - buf, 1, stream);

	return dest;
}

void
dawn_sethi_ullman(struct tile *tile, void *stream)
{
	calc_reg_usage(tile);
	sethi_ullman_rec(tile, 0, stream);
}

