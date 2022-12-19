#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

lex()
{
}

expr_nuds[] = {
};

expr_leds[] = {
};

static void
pexpr()
{
}

parse()
{
}

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
#define INS_IMMED_WIDTH(ins) ((ins)->immed_8 ? 0 : ((ins)->pfxs & PFX_ADDRSZ ? 1 : ((ins)->pfxs & PFX_REX_W ? 3 : 2)))

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

	uint8_t mod;
	uint8_t reg;
	uint8_t rm;

	uint8_t base;
	uint8_t index;
	uint8_t scale;

	uint64_t disp;
	uint64_t immed;
};


static void
emit_prefixes(uint16_t prefixes, uint8_t **O)
{
	if (prefixes & PFX_FS) *(*O)++ = 0x64;
	if (prefixes & PFX_GS) *(*O)++ = 0x65;
	if (prefixes & PFX_DATASZ) *(*O)++ = 0x66;
	if (prefixes & PFX_ADDRSZ) *(*O)++ = 0x67;

	if (prefixes & PFX_REX_W) *(*O)++ = 0x48;
	
	if (prefixes & PFX_0F) *(*O)++ = 0x0F;
}

static void
emit_immed(int64_t immed, int width, uint8_t **O)
{
	for (int i = 0; i < 1 << width; i++) {
		*(*O)++ = immed;
		immed >>= 8;
	}
}

static int
emit(const struct ins *ins, size_t offset, uint8_t **O)
{
	if (ins->pfxs) emit_prefixes(ins->pfxs, O);
	*(*O)++ = ins->opcode;
	if (INS_HAS_MODRM(ins)) {
		*(*O)++ = (ins->mod<<6) | (ins->reg<<3) | ins->rm;
	}
	if (INS_HAS_SIB(ins)) {
		*(*O)++ = (ins->scale<<6) | (ins->index<<3) | ins->base;
	}
	if (INS_HAS_DISP(ins)) {
		if (ins->disp.type == EXPR_REL) {
			/* TODO proper offset! */
			if (current_pass == 2) add_reloc(offset, ins->disp.symbol, ELF_R_X86_64_PC32, ins->disp.value - 4);
			emit_immed(0, 2, O);
		} else {
			emit_immed(ins->disp.value, 2, O);
		}
	}
	if (INS_HAS_IMMED(ins)) {
		if (ins->immed.type == EXPR_REL) {
			/* TODO proper offset? */
			if (current_pass == 2) add_reloc(offset, ins->immed.symbol, ELF_R_X86_64_PC32, ins->immed.value - 4);
			emit_immed(0, 2, O);
		} else {
			emit_immed(ins->immed.value, INS_IMMED_WIDTH(ins), O);
		}
	}
	return 0;
}

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	printf("hi\n");
	return 0;
}

