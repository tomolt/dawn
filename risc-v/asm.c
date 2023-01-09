#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "ins.h"

void
riscv_assemble(const struct template *template, const size_t *bindings, void *file)
{
	(void)bindings;

	for (int i = 0; i < template->num_ins; i++) {
		const struct riscv_ins *ins = &template->ins[i];

		uint32_t raw_ins = 0;

		raw_ins |= ins->opcode & 0x3F;
		switch (ins->fmt) {
		case FMT_R:
			raw_ins |= (ins->rd & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (ins->rs1 & 0x1F) << 15;
			raw_ins |= (ins->rs2 & 0x1F) << 20;
			raw_ins |= (ins->funct7 & 0x7F) << 25;
			break;
		case FMT_I:
			raw_ins |= (ins->rd & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (ins->rs1 & 0x1F) << 15;
			raw_ins |= (ins->imm & 0xFFF) << 20;
			break;
		case FMT_S:
			raw_ins |= (ins->imm & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (ins->rs1 & 0x1F) << 15;
			raw_ins |= (ins->rs2 & 0x1F) << 20;
			raw_ins |= ((ins->imm >> 5) & 0x7F) << 25;
			break;
		case FMT_U:
			raw_ins |= (ins->rd & 0x1F) << 7;
			raw_ins |= ((ins->imm >> 12) & 0xFFFFF) << 12;
			break;
		}

		fwrite(&raw_ins, 1, 4, file);
	}
}

