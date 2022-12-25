#define REG_AX	0
#define REG_CX	1
#define REG_SP	4
#define REG_BP	5

#define PFX_0F		0x1
#define PFX_REX		0x2
#define PFX_REX_W	0x4
#define PFX_REX_R	0x8
#define PFX_REX_X	0x10
#define PFX_REX_B	0x20
#define PFX_REX_MASK	(PFX_REX|PFX_REX_W|PFX_REX_R|PFX_REX_X|PFX_REX_B)
#define PFX_DATASZ	0x40
#define PFX_ADDRSZ	0x80
#define PFX_FS		0x100
#define PFX_GS		0x200

#define HAS_MODRM	0x0100
#define HAS_IMMED	0x0200
#define SMALL_IMMED	0x0400

#define INS_HAS_MODRM(ins) ((ins)->opcode & HAS_MODRM)
#define INS_HAS_SIB(ins) ((ins)->rm == REG_SP)
#define INS_HAS_DISP(ins) ((ins)->mod == 1 || (ins)->mod == 2 || (!(ins)->mod && (ins)->rm == REG_BP))
#define INS_DISP_WIDTH(ins) ((ins)->mod == 1 ? 0 : 2)
#define INS_HAS_IMMED(ins) ((ins)->opcode & HAS_IMMED)
#define INS_IMMED_WIDTH(ins) ((ins)->opcode & SMALL_IMMED ? 0 : ((ins)->prefixes & PFX_ADDRSZ ? 1 : ((ins)->prefixes & PFX_REX_W ? 3 : 2)))

#define MOD_MEM_ND	0
#define MOD_MEM_SD	1
#define MOD_MEM_LD	2
#define MOD_REG		3

struct ins {
	uint16_t prefixes;
	uint16_t opcode;

	uint8_t mod;
	uint8_t reg;
	uint8_t rm;

	uint8_t base;
	uint8_t index;
	uint8_t scale;

	int64_t disp;
	int64_t immed;
};

struct tile {
	int opclass;
	int opnum;

	int64_t immed;

	int maxregs;
	int holdregs;
	int spill;

	int arity;
	struct tile *operands[];
};

#define OPCL_ARITH_RM	0x03
#define OPCL_ARITH_MI	0x81
#define OPCL_SHIFT_MC	0xD3
#define OPCL_SHIFT_MI	0xC1
#define OPCL_MOV_EI	0xB8
#define OPCL_MOV_RM	0x8B
#define OPCL_MUL_M	0xF7

#define OPNO_ADD	0x00
#define OPNO_SUB	0x05
#define OPNO_AND	0x04
#define OPNO_OR		0x01
#define OPNO_XOR	0x06

#define OPNO_SHL	0x04
#define OPNO_SHR	0x05
#define OPNO_SAR	0x07

#define OPNO_NOT	0x02
#define OPNO_NEG	0x03

