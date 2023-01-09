#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "ins.h"
#include "../muop.h"

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof*(array))

#define LUI	0x37
#define AR      0x33    // Arithmetic 3-register operation
#define AI      0x13    // Arithmetic 2-register, 1-immediate operation

#define ADD	0
#define AND	7
#define OR	6
#define XOR	4

#define BEGIN_TPL(name)						\
	const struct riscv_ins riscv_ins_##name[] = {
#define CON(...)
#define R(opcode, rd, funct3, rs1, ...)				\
	{ 0, opcode, rd, funct3, rs1, __VA_ARGS__, .fmt=FMT_R },
#define I(opcode, rd, funct3, rs1, imm)				\
	{ imm, opcode, rd, funct3, rs1, .fmt=FMT_I },
#define S(opcode, funct3, rs1, rs2, imm)			\
	{ imm, opcode, rd, funct3, rs1, rs2, .fmt=FMT_S },
#define U(opcode, rd, imm)					\
	{ imm, opcode, rd, .fmt=FMT_U },
#define X(n)		(0x80+(n))
#define END_TPL()	};
#include "templates.def"
#undef BEGIN_TPL
#undef CON
#undef R
#undef I
#undef S
#undef U
#undef X
#undef END_TPL

#define BEGIN_TPL(name)						\
	const struct template riscv_tpl_##name = {	\
		ARRAY_LENGTH(riscv_ins_##name),			\
		riscv_ins_##name,
#define CON(...)	(char[]){ __VA_ARGS__, 0 },
#define END_TPL()	};
#define R(opcode, rd, funct3, rs1, ...)
#define I(opcode, rd, funct3, rs1, imm)
#define S(opcode, funct3, rs1, rs2, imm)
#define U(opcode, rd, imm)
#define X(n)	0
#include "templates.def"
#undef BEGIN_TPL
#undef CON
#undef R
#undef I
#undef S
#undef U
#undef X
#undef END_TPL

const struct template *
riscv_tile(const struct museq *museq, size_t index, size_t *bindings)
{
	int32_t imm;
	const struct muop *muop = &museq->muops[index];
	switch (muop->op) {
#if 0
	case MU_NEG:
		// sub rd, x0, rs2
		break;

	case MU_NOT:
		// xori rd, rs1, -1
		break;
#endif

	case MU_ADD:
		bindings[0] = index;
		bindings[1] = index - muop->arg1;
		bindings[2] = index - muop->arg2;
		return &riscv_tpl_add_r;

	case MU_IMM:
		imm   = muop->arg2;
		imm <<= 16;
		imm  |= muop->arg1;
		bindings[0] = index;
		bindings[1] = imm & 0x00000FFF;
		bindings[2] = imm & 0xFFFFF000;
		if (imm >= -2048 && imm <= 2047) {
			return &riscv_tpl_li_small;
		} else {
			return &riscv_tpl_li_large;
		}

	default:
		assert(0);
		return NULL;
	}
}

