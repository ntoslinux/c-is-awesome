/*
 * Liveness analysis
 * Copyright (c) 2007,2009  Pekka Enberg
 *
 * This file is released under the GPL version 2 with the following
 * clarification and special exception:
 *
 *     Linking this library statically or dynamically with other modules is
 *     making a combined work based on this library. Thus, the terms and
 *     conditions of the GNU General Public License cover the whole
 *     combination.
 *
 *     As a special exception, the copyright holders of this library give you
 *     permission to link this library with independent modules to produce an
 *     executable, regardless of the license terms of these independent
 *     modules, and to copy and distribute the resulting executable under terms
 *     of your choice, provided that you also meet, for each linked independent
 *     module, the terms and conditions of the license of that module. An
 *     independent module is a module which is not derived from or based on
 *     this library. If you modify this library, you may extend this exception
 *     to your version of the library, but you are not obligated to do so. If
 *     you do not wish to do so, delete this exception statement from your
 *     version.
 *
 * Please refer to the file LICENSE for details.
 */

#include "jit/compilation-unit.h"
#include "jit/compiler.h"
#include "jit/instruction.h"
#include "jit/ssa.h"
#include "jit/vars.h"

#include "lib/bitset.h"
#include "vm/die.h"

#include <errno.h>
#include <stdlib.h>

static void __update_live_ranges(struct compilation_unit *cu, struct basic_block *bb)
{
	struct var_info *uses[MAX_REG_OPERANDS];
	struct var_info *defs[MAX_REG_OPERANDS];
	struct insn *insn;
	unsigned int nr_uses;
	unsigned int nr_defs;
	unsigned int i;

	for_each_insn_reverse(insn, &bb->insn_list) {
		nr_defs = insn_defs(bb->b_parent, insn, defs);
		for (i = 0; i < nr_defs; i++) {
			if (interval_is_empty(defs[i]->interval)) {
				interval_add_range(cu, defs[i]->interval, insn->lir_pos + 1, bb->end_insn);
				continue;
			}

			struct live_range *r
				= interval_first_range(defs[i]->interval);

			r->start = insn->lir_pos + 1;
		}

		nr_uses = insn_uses(insn, uses);
		for (i = 0; i < nr_uses; i++)
			interval_add_range(cu, uses[i]->interval, bb->start_insn, insn->lir_pos + 1);

		if (insn_is_call(insn)) {
			for (i = 0; i < NR_CALLER_SAVE_REGS; i++) {
				struct var_info *var;

				var = get_fixed_var(cu, caller_save_regs[i]);

				interval_add_range(cu, var->interval, insn->lir_pos + 1, insn->lir_pos + 2);
			}
		}
	}
}

static void update_live_ranges(struct compilation_unit *cu)
{
	struct basic_block *this;

	for_each_basic_block_reverse(this, &cu->bb_list) {
		struct var_info *var;

		for_each_variable(var, cu->var_infos) {
			if (test_bit(this->live_out_set->bits, var->vreg))
				interval_add_range(cu, var->interval, this->start_insn, this->end_insn);
		}

		__update_live_ranges(cu, this);
	}
}

int analyze_live_sets(struct compilation_unit *cu)
{
	struct bitset *old_live_out_set = NULL;
	struct bitset *old_live_in_set = NULL;
	struct basic_block *this;
	bool changed;
	int err = 0;

	old_live_in_set = alloc_bitset(cu->nr_vregs);
	if (!old_live_in_set) {
		err = -ENOMEM;
		goto out;
	}

	old_live_out_set = alloc_bitset(cu->nr_vregs);
	if (!old_live_out_set) {
		err = -ENOMEM;
		goto out;
	}

	/*
	 * This is your standard text-book iterative liveness analysis
	 * algorithm for computing live-in and live-out sets for each
	 * basic block.
	 */
	do {
		changed = false;

		for_each_basic_block_reverse(this, &cu->bb_list) {
			struct basic_block *succ;
			unsigned long i;

			bitset_copy_to(this->live_in_set, old_live_in_set);
			bitset_copy_to(this->live_out_set, old_live_out_set);

			bitset_copy_to(this->live_out_set, this->live_in_set);
			bitset_sub(this->def_set, this->live_in_set);
			bitset_union_to(this->use_set, this->live_in_set);

			bitset_clear_all(this->live_out_set);
			for (i = 0; i < this->nr_successors; i++) {
				succ = this->successors[i];
				bitset_union_to(succ->live_in_set, this->live_out_set);
			}

			/*
			 * If live-in or live-out set changed, we must
			 * iterate at least once more.
			 */
			if (!bitset_equal(this->live_in_set, old_live_in_set) ||
			    !bitset_equal(this->live_out_set, old_live_out_set))
				changed = true;
		}
	} while (changed);
  out:
	free(old_live_out_set);
	free(old_live_in_set);
	return err;
}

static void __analyze_use_def(struct basic_block *bb, struct insn *insn)
{
	struct var_info *uses[MAX_REG_OPERANDS];
	struct var_info *defs[MAX_REG_OPERANDS];
	int nr_uses;
	int nr_defs;
	int i;

	nr_uses = insn_uses(insn, uses);
	for (i = 0; i < nr_uses; i++) {
		struct var_info *var = uses[i];

		/*
		 * It's in the use set if and only if it has not
		 * _already_ been defined by insn basic block.
		 */
		if (!test_bit(bb->def_set->bits, var->vreg))
			set_bit(bb->use_set->bits, var->vreg);
	}

	nr_defs = insn_defs(bb->b_parent, insn, defs);
	for (i = 0; i < nr_defs; i++) {
		struct var_info *var = defs[i];

		set_bit(bb->def_set->bits, var->vreg);
	}
}

void analyze_use_def(struct compilation_unit *cu)
{
	struct basic_block *bb;

	for_each_basic_block(bb, &cu->bb_list) {
		struct insn *insn;

		for_each_insn(insn, &bb->insn_list) {
			__analyze_use_def(bb, insn);
		}
	}
}

static int __init_sets(struct basic_block *bb, unsigned long nr_vregs)
{
	bb->use_set = alloc_bitset(nr_vregs);
	if (!bb->use_set)
		return warn("out of memory"), -ENOMEM;

	bb->def_set = alloc_bitset(nr_vregs);
	if (!bb->def_set)
		return warn("out of memory"), -ENOMEM;

	bb->live_in_set = alloc_bitset(nr_vregs);
	if (!bb->live_in_set)
		return warn("out of memory"), -ENOMEM;

	bb->live_out_set = alloc_bitset(nr_vregs);
	if (!bb->live_out_set)
		return warn("out of memory"), -ENOMEM;

	return 0;
}

int init_sets(struct compilation_unit *cu)
{
	struct basic_block *this;
	int err = 0;

	for_each_basic_block(this, &cu->bb_list) {
		err = __init_sets(this, cu->nr_vregs);
		if (err)
			break;
	}
	return err;
}

int analyze_liveness(struct compilation_unit *cu)
{
	int err = 0;

	err = init_sets(cu);
	if (err)
		goto out;

	analyze_use_def(cu);

	err = analyze_live_sets(cu);
	if (err)
		goto out;

	update_live_ranges(cu);

  out:
	return err;
}
