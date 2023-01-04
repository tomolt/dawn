
/*
 * - Near-perfect register allocation for linear blocks
 * - lifetime splitting / second-chance allocation
 * - x86 instruction selection with operand fusion
 * - dead-code elimination
 * - spill-code insertion and fusion
 * - assembling
 * - rematerialization
 */

enum {
	LOC_REGISTER,
	LOC_STACK
};

struct loc {
};

struct reg {
	size_t inhabitant;
	size_t next_use;
};

struct def {
	size_t arg;
	//uint8_t fixed_reg;
	//size_t follows_arg;
};

struct use {
	size_t arg;
	//uint8_t fixed_reg;
};

struct tile {
	struct tile *prev;
	struct tile *next;
	struct use uses[];
	struct def defs[];
};

struct compiler {
};

calc_usecounts()
{
	for (size_t i = museq->count; i--;) {
		if (has_side_effects(op)) {
			usecounts[i]++;
		}
		switch (num_dependencies(op)) {
		case 2:
			usecounts[i - op->arg2]++;
			/* fallthrough */
		case 1:
			usecounts[i - op->arg1]++;
		}
	}
}

drop_deps()
{
	switch (num_dependencies(op)) {
	case 2:
		usecounts[i - op->arg2]--;
		/* fallthrough */
	case 1:
		usecounts[i - op->arg1]--;
	}
}

transfer()
{
}

select_reg()
{
	if (fixed_reg) {
		return fixed_reg;
	}

	for () {
		if (available) {
			return reg;
		}
	}

	candidate;
	for () {
		if (regs[reg].next_use > regs[candidate].next_use) {
			candidate = reg;
		}
	}

	return candidate;
}

void
compile(const struct museq *seq, uint16_t *usecounts)
{
	uint8_t *which_reg;
	for (size_t cursor = seq->count; cursor--;) {
		const struct muop *op = &seq->muops[cursor];

		if (!usecounts[cursor]) {
			drop_deps();
			continue;
		}

		cover();

		for (uses) {
			if (in_reg(use.arg)) {
				reg = ;
			} else {
				/* arg is apparently not needed after the current instruction.
				 * it may therefore overlap with any definitions. */
				reg = select_reg();
			}
		}

		for (defs) {
			if (!in_reg(def.arg)) {
				reg = select_reg();
				transfer();
			} else {
				reg = ;
			}
		}

		build_ins();
		encode_ins();
		prepend_ins();

		for (defs) {
			release_reg();
		}

		for (uses) {
			if (!in_reg(use.arg)) {
				unspill();
			}
		}
	}
}
