#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ins.h"
#include "../revbuf.h"

static void
revbuf_prepend(struct revbuf *revbuf, const void *data, size_t size)
{
	if (revbuf->start < size) {
		if (!revbuf->capac) {
			revbuf->capac = 256;
			revbuf->start = revbuf->capac;
			revbuf->data = malloc(revbuf->capac);
		} else {
			void *new_data = malloc(2 * revbuf->capac);
			memcpy(new_data, revbuf->data, revbuf->capac);
			free(revbuf->data);
			revbuf->start += revbuf->capac;
			revbuf->capac += revbuf->capac;
			revbuf->data = new_data;
		}
	}
	revbuf->start -= size;
	memcpy(revbuf->data + revbuf->start, data, size);
}

void
riscv_assemble(const struct template *template, const size_t *assignments, struct revbuf *revbuf)
{
	char buffer[4*template->num_ins];
	for (int i = 0; i < template->num_ins; i++) {
		const struct riscv_ins *ins = &template->ins[i];

		int8_t rd = ins->rd;
		if (rd & 0x80) rd = assignments[rd&0x7F];
		int8_t rs1 = ins->rs1;
		if (rs1 & 0x80) rs1 = assignments[rs1&0x7F];
		int8_t rs2 = ins->rs2;
		if (rs2 & 0x80) rs2 = assignments[rs2&0x7F];
		// FIXME this is probably pretty stupid
		int32_t imm = ins->imm;
		if (imm & 0x80) imm = (int32_t)assignments[imm&0x7F];

		uint32_t raw_ins = 0;

		raw_ins |= ins->opcode & 0x3F;
		switch (ins->fmt) {
		case FMT_R:
			raw_ins |= (rd & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (rs1 & 0x1F) << 15;
			raw_ins |= (rs2 & 0x1F) << 20;
			raw_ins |= (ins->funct7 & 0x7F) << 25;
			break;
		case FMT_I:
			raw_ins |= (rd & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (rs1 & 0x1F) << 15;
			raw_ins |= (imm & 0xFFF) << 20;
			break;
		case FMT_S:
			raw_ins |= (imm & 0x1F) << 7;
			raw_ins |= (ins->funct3 & 0x7) << 12;
			raw_ins |= (rs1 & 0x1F) << 15;
			raw_ins |= (rs2 & 0x1F) << 20;
			raw_ins |= ((imm >> 5) & 0x7F) << 25;
			break;
		case FMT_U:
			raw_ins |= (rd & 0x1F) << 7;
			raw_ins |= ((imm >> 12) & 0xFFFFF) << 12;
			break;
		}

		memcpy(buffer + 4*i, &raw_ins, 4);
	}
	revbuf_prepend(revbuf, buffer, 4*template->num_ins);
}

