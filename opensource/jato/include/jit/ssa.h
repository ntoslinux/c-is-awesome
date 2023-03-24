#ifndef JATO__JIT_SSA_H
#define JATO__JIT_SSA_H

#include "lib/hash-map.h"

/*
 * The work and inserted arrays from the LIR
 * to SSA algorithm initially contain no block.
 * We mark this special block as SSA_INIT_BLOCK.
 */
#define SSA_INIT_BLOCK -1

struct changed_var_stack {
	unsigned long vreg;
	struct changed_var_stack *next;
};

struct loop_worklist {
	struct basic_block *bb;
	struct loop_worklist *next;
};

struct dce {
	struct var_info *var;
	struct dce *next;
};

struct array_size_acc {
	bool active;
	long size;
};

struct reg_value_acc {
	bool active;
	long val;
};

/*
 * Functions defined in jit/ssa.c
 */
void recompute_insn_positions(struct compilation_unit *);
void remove_insn(struct insn *insn);

/*
 * Functions defined in jit/liveness.c
 */

int init_sets(struct compilation_unit *);
void analyze_use_def(struct compilation_unit *);
int analyze_live_sets(struct compilation_unit *);

#endif
