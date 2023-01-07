
enum {
	X86_ARITH_RM,
	X86_ARITH_MI,
	X86_SHIFT_MC,
	X86_SHIFT_MI,
	X86_MOV_RM,
	X86_MOV_EI,
	X86_MUL_M,
};

tile()
{
	switch (op) {
	case MU_COPY:
		X86_MOV_RM;
		break;

	case MU_NEG: case MU_NOT:
		break;

	case MU_ADD: case MU_SUB:
	case MU_AND: case MU_OR: case MU_XOR:
		X86_ARITH_RM;
		break;

	case MU_MUL:
		break;

	case MU_DIV: case MU_MOD:
		break;

	case MU_SHL: case MU_SHR:
		X86_SHIFT_MC;
		break;

	case MU_LDL:
		X86_MOV_RM;
		break;

	case MU_IMM:
		X86_MOV_EI;
		break;
	}
}

tile_add_group()
{
}

tile_shift_group()
{
}


