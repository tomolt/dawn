enum { DEF=1, USE, IMM };

enum { FMT_R, FMT_I, FMT_S, FMT_U };

struct riscv_ins {
	int32_t imm;
	uint8_t opcode;
	uint8_t rd;
	uint8_t funct3;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t funct7;
	uint8_t fmt;
};

struct template {
	int num_ins;
	const struct riscv_ins *ins;
	const char *constraints;
};

