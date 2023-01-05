#define MU_COPY 0x00
#define MU_NEG  0x10
#define MU_NOT  0x11
#define MU_ADD  0x20
#define MU_SUB  0x21
#define MU_AND  0x22
#define MU_OR   0x23
#define MU_XOR  0x24
#define MU_MUL  0x25
#define MU_DIV  0x26
#define MU_MOD  0x27
#define MU_SHL  0x28
#define MU_SHR  0x29
#define MU_LDL  0x30
#define MU_STL  0x31
#define MU_LDR  0x32
#define MU_STR  0x33
#define MU_IMM  0x40

struct muop {
	uint8_t  op;
	uint16_t arg1;
	uint16_t arg2;
};

struct museq {
	struct muop *muops;
	size_t count;
	size_t capac;
};

size_t museq_append(struct museq *museq, uint8_t op, size_t arg1, size_t arg2);
size_t museq_append_imm(struct museq *museq, int32_t value);
