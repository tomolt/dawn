#include <stdint.h>

#include "ins.h"

enum { DEF, USE, IMM };

#define LUI	0x37
#define AR      0x33    // Arithmetic 3-register operation
#define AI      0x13    // Arithmetic 2-register, 1-immediate operation

#define ADD	0
#define AND	7
#define OR	6
#define XOR	4

#define BEGIN_TPL(name)	\
	const struct riscv_template riscv_tpl_##name = {
#define END_TPL()	}};
#define CON(...)	{ __VA_ARGS__, 0 }, {
#define R(opcode, rd, funct3, rs1, ...)		\
	(struct riscv_ins){ 0, opcode, rd, funct3, rs1, __VA_ARGS__, .fmt=FMT_R },
#define I(opcode, rd, funct3, rs1, imm)		\
	(struct riscv_ins){ imm, opcode, rd, funct3, rs1, .fmt=FMT_I },
#define S(opcode, funct3, rs1, rs2, imm)	\
	(struct riscv_ins){ imm, opcode, rd, funct3, rs1, rs2, .fmt=FMT_S },
#define U(opcode, rd, imm)			\
	(struct riscv_ins){ imm, opcode, rd, .fmt=FMT_U },
#define X(n)		(0x80+(n))
#include "templates.def"
#undef BEGIN_TPL
#undef END_TPL
#undef CON
#undef R
#undef I
#undef S
#undef U
#undef X

#if 0
static struct tile
tile_arith(int )
{
	if (muop_commutative() && museq->muops[] == MU_IMM) {
	}

	if (seq->muops[] == MU_IMM) {

		imm = ;
		opcode = 0x13;
		funct3 = op;

		if (op == SUB) {
			opcode = ADD;
			imm = -imm;
		}
	} else {
		opcode = 0x33;
		funct3 = op;

		add_use();
	}

	add_use();
	add_def();
}

struct tile
riscv_tile()
{
	switch () {
	case MU_COPY:
		// add rd, x0, rs2
		break;

	case MU_NEG:
		// sub rd, x0, rs2
		break;

	case MU_NOT:
		// xori rd, rs1, -1
		break;

	case MU_ADD: return tile_arith(RV32I_ADD);
	case MU_SUB: return tile_arith(RV32I_SUB);
	case MU_AND: return tile_arith(RV32I_AND);
	case MU_OR:  return tile_arith(RV32I_OR);
	case MU_XOR: return tile_arith(RV32I_XOR);
	case MU_MUL: return tile_arith();
	case MU_DIV: return tile_arith();
	case MU_MOD: return tile_arith();

	case MU_IMM:
		if (imm >= -2048 && imm <= 2047) {
			// addi rd, x0, imm
		} else {
			// lui  rd, ((imm>>12) + ((imm>>11)&1))
			// addi rd, rd, (imm)
		}
		break;

	}
}
#endif

