#define NO_VAR SIZE_MAX
#define NUM_REGISTERS 16

enum {
	LOC_NONEXISTENT = 0,
	LOC_REGISTER,
	LOC_STACK
};

struct loc {
	int    kind;
	size_t address;
};

struct reg {
	size_t var;
	bool   available;
};

struct var {
	struct loc spill;
	size_t next_use;
	int reg;
};

struct constraint {
	size_t var;
	struct loc loc;
};

struct tile {
	struct tile *next;
	int template;
	int op;
	int num_defs;
	int num_uses;
	struct constraint constraints[];
};

struct compiler {
	struct reg regs[NUM_REGISTERS];
	struct var *vars;
	size_t index;
};

