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
#define PFX_DATASZ	0x40
#define PFX_ADDRSZ	0x80
#define PFX_FS		0x100
#define PFX_GS		0x200

#define INS_HAS_MODRM(ins) ((ins)->has_modrm)
#define INS_HAS_SIB(ins) ((ins)->rm == REG_SP)
#define INS_HAS_DISP(ins) ((ins)->mod == 1 || (ins)->mod == 2 || (!(ins)->mod && (ins)->rm == REG_BP))
//#define INS_DISP_WIDTH(ins)
#define INS_HAS_IMMED(ins) ((ins)->has_immed)
#define INS_IMMED_WIDTH(ins) ((ins)->small_immed ? 0 : ((ins)->prefixes & PFX_ADDRSZ ? 1 : ((ins)->prefixes & PFX_REX_W ? 3 : 2)))

#define MOD_MEM_ND	0
#define MOD_MEM_SD	1
#define MOD_MEM_LD	2
#define MOD_REG		3

struct ins {
	uint16_t prefixes;
	uint8_t  opcode;

	bool has_modrm;
	bool has_immed;
	bool small_immed;
	bool embedded_reg;

	uint8_t mod;
	uint8_t reg;
	uint8_t rm;

	uint8_t base;
	uint8_t index;
	uint8_t scale;

	uint64_t disp;
	uint64_t immed;

	int arity;
	struct ins *operands[];
};

#define OPC_MOV_RI	0xB8
#define OPC_ADD_RR	0x01
#define OPC_SUB_RR	0x29

