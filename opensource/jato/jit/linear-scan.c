/*
 * Linear scan register allocation
 * Copyright © 2007-2008  Pekka Enberg
 * Copyright © 2008	  Arthur Huillet
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
 *
 * For more details on the linear scan register allocation algorithm used here,
 * please refer to the following paper:
 *
 *   Wimmer, C. and Mössenböck, H. 2005. Optimized interval splitting
 *   in a linear scan register allocator. In Proceedings of the 1st
 *   ACM/USENIX international Conference on Virtual Execution
 *   Environments (Chicago, IL, USA, June 11 - 12, 2005). VEE '05. ACM
 *   Press, New York, NY, 132-141.
 */

#include "jit/compiler.h"
#include "jit/vars.h"

#include "lib/bitset.h"
#include "lib/pqueue.h"

#include "vm/die.h"

#include <limits.h>
#include <stdlib.h>
#include <errno.h>

static void set_free_pos(unsigned long *free_until_pos, enum machine_reg reg,
			 unsigned long pos)
{
	/*
	 * If reg is greater than or equal to NR_REGISTERS, it's
	 * not a general purpose register and thus not available for
	 * allocation so it's safe to just ignore it.
	 */
	if (reg >= NR_REGISTERS)
		return;

	/*
	 * If the position for one register is set multiple times the minimum
	 * of all positions is used. This can happen when many inactive
	 * intervals have the same register assigned.
	 */
	if (free_until_pos[reg] < pos)
		return;

	free_until_pos[reg] = pos;
}

static void set_use_pos(unsigned long *use_pos, enum machine_reg reg,
			unsigned long pos)
{
	/*
	 * This function does the same as set_free_pos so we call this directly
	 */
	assert(reg < NR_REGISTERS);
	set_free_pos(use_pos, reg, pos);
}

static void set_block_pos(unsigned long *block_pos, unsigned long *use_pos,
			  enum machine_reg reg, unsigned long pos)
{
	/*
	 * This function does the same as set_free_pos so we call this directly
	 */
	assert(reg < NR_REGISTERS);
	set_free_pos(block_pos, reg, pos);
	set_free_pos(use_pos, reg, pos);
}

/* Find the register that is used the latest.  */
static enum machine_reg pick_register(unsigned long *free_until_pos, enum vm_type type)
{
	unsigned long max_pos = 0;
	int ret = -1;

	for (unsigned int i = 0; i < NR_REGISTERS; i++) {
		unsigned long pos = free_until_pos[i];

		if (!reg_supports_type(i, type))
			continue;

		if (pos >= max_pos) {
			max_pos = pos;
			ret = i;
		}
	}

	assert(ret != -1);
	return ret;
}

static void spill_interval(struct compilation_unit *cu, struct live_interval *it, unsigned long pos, struct pqueue *unhandled)
{
	struct live_interval *new;

	new = split_interval_at(cu, it, pos);
	if (has_use_positions(new)) {
		unsigned long next_pos = next_use_pos(new, 0);

		/* Trim interval if it does not start with a use position. */
		if (next_pos > interval_start(new))
			new = split_interval_at(cu, new, next_pos);

		/*
		 * If any child interval of @it must be reloaded from
		 * @it then we have to update its spill parent to @new.
		 */
		struct live_interval *child = new->next_child;
		while (child) {
			if (interval_needs_reload(child) &&
			    child->spill_parent == it)
				child->spill_parent = new;

			child = child->next_child;
		}

		if (interval_needs_spill(it))
			mark_need_spill(new);

		/*
		 * When next use position is a write then we must not
		 * reload the new interval. One reason for this is
		 * that it's unnecessary. Another one is that we won't
		 * be able to insert a reload instruction in the
		 * middle of instruction when new interval starts at odd
		 * position.
		 */
		if ((next_pos & 1) == 0)
			mark_need_reload(new, it);

		mark_need_spill(it);
		pqueue_insert(unhandled, interval_start(new), new);
	}
}

static void __spill_interval_intersecting(struct compilation_unit *cu,
					  struct live_interval *current,
					  enum machine_reg reg,
					  struct live_interval *it,
					  struct pqueue *unhandled)
{
	unsigned long start;

	if (it->reg != (int8_t) reg)
		return;

	if (!intervals_intersect(it, current))
		return;

	start = interval_intersection_start(current, it);
	if (start == interval_start(it))
		return;

	spill_interval(cu, it, start, unhandled);
}

static void spill_all_intervals_intersecting(struct compilation_unit *cu,
					     struct live_interval *current,
					     enum machine_reg reg,
					     struct list_head *active,
					     struct list_head *inactive,
					     struct pqueue *unhandled)
{
	struct live_interval *it;

	list_for_each_entry(it, active, interval_node) {
		__spill_interval_intersecting(cu, current, reg, it, unhandled);
	}

	list_for_each_entry(it, inactive, interval_node) {
		__spill_interval_intersecting(cu, current, reg, it, unhandled);
	}

}

static void allocate_blocked_reg(struct compilation_unit *cu,
				 struct live_interval *current,
				 struct list_head *active,
				 struct list_head *inactive,
				 struct pqueue *unhandled)
{
	unsigned long use_pos[NR_REGISTERS], block_pos[NR_REGISTERS];
	struct live_interval *it;
	int i;
	enum machine_reg reg;

	for (i = 0; i < NR_REGISTERS; i++) {
		use_pos[i] = LONG_MAX;
		block_pos[i] = LONG_MAX;
	}

	list_for_each_entry(it, active, interval_node) {
		unsigned long pos;

		if (interval_has_fixed_reg(it))
			continue;

		pos = next_use_pos(it, interval_start(current));
		set_use_pos(use_pos, it->reg, pos);
	}

	list_for_each_entry(it, inactive, interval_node) {
		unsigned long pos;

		if (interval_has_fixed_reg(it))
			continue;

		if (!reg_supports_type(it->reg, current->var_info->vm_type))
			continue;

		if (intervals_intersect(it, current)) {
			pos = next_use_pos(it, interval_start(current));
			set_use_pos(use_pos, it->reg, pos);
		}
	}

	list_for_each_entry(it, active, interval_node) {
		if (!interval_has_fixed_reg(it))
			continue;

		set_block_pos(block_pos, use_pos, it->reg, 0);
	}

	list_for_each_entry(it, inactive, interval_node) {
		if (!interval_has_fixed_reg(it))
			continue;

		if (!reg_supports_type(it->reg, current->var_info->vm_type))
			continue;

		if (intervals_intersect(it, current)) {
			unsigned long pos;

			pos = interval_intersection_start(it, current);
			set_block_pos(block_pos, use_pos, it->reg, pos);
		}
	}

	reg = pick_register(use_pos, current->var_info->vm_type);
	if (use_pos[reg] < next_use_pos(current, 0)) {
		unsigned long pos;

		/*
		 * All active and inactive intervals are used before current,
		 * so it is best to spill current itself
		 */
		pos = next_use_pos(current, interval_start(current));
		spill_interval(cu, current, pos, unhandled);
	} else {
		/*
		 * Register is available for whole or some part of interval
		 */
		current->reg = reg;

		if (block_pos[reg] < interval_end(current))
			spill_interval(cu, current, block_pos[reg], unhandled);

		spill_all_intervals_intersecting(cu, current, reg, active,
						 inactive, unhandled);
	}
}

static void try_to_allocate_free_reg(struct compilation_unit *cu,
				     struct live_interval *current,
				     struct list_head *active,
				     struct list_head *inactive,
				     struct pqueue *unhandled)
{
	unsigned long free_until_pos[NR_REGISTERS];
	struct live_interval *it;
	enum machine_reg reg;
	int i;

	for (i = 0; i < NR_REGISTERS; i++)
		free_until_pos[i] = LONG_MAX;

	list_for_each_entry(it, active, interval_node) {
		set_free_pos(free_until_pos, it->reg, 0);
	}

	list_for_each_entry(it, inactive, interval_node) {
		if (!reg_supports_type(it->reg, current->var_info->vm_type))
			continue;

		if (intervals_intersect(it, current)) {
			unsigned long pos;

			pos = interval_intersection_start(it, current);
			set_free_pos(free_until_pos, it->reg, pos);
		}
	}

	reg = pick_register(free_until_pos, current->var_info->vm_type);
	if (free_until_pos[reg] == 0) {
		/*
		 * No register available without spilling.
		 */
		return;
	}

	if (interval_end(current) <= free_until_pos[reg]) {
		/*
		 * Register available for the full interval.
		 */
		current->reg = reg;
	} else {
		/*
		 * Register available for the first part of the interval.
		 */
		spill_interval(cu, current, free_until_pos[reg], unhandled);
		current->reg = reg;
	}
}

int allocate_registers(struct compilation_unit *cu)
{
	struct list_head inactive = LIST_HEAD_INIT(inactive);
	struct list_head active = LIST_HEAD_INIT(active);
	struct live_interval *current;
	struct pqueue *unhandled;
	struct bitset *registers;
	struct var_info *var;

	registers = alloc_bitset(NR_REGISTERS);
	if (!registers)
		return warn("out of memory"), -ENOMEM;

	bitset_set_all(registers);

	unhandled = pqueue_alloc();
	if (!unhandled) {
		free(registers);
		return warn("out of memory"), -ENOMEM;
	}

	/*
	 * Fixed intervals are placed on the inactive list initially so that
	 * the allocator can avoid conflicts when allocating a register for a
	 * non-fixed interval.
	 */
	for_each_variable(var, cu->var_infos) {
		if (interval_is_empty(var->interval))
			continue;

		if (interval_has_fixed_reg(var->interval)) {
			if (var->interval->reg < NR_REGISTERS)
				list_add(&var->interval->interval_node, &inactive);
		} else
			pqueue_insert(unhandled, interval_start(var->interval), var->interval);
	}

	while (!pqueue_is_empty(unhandled)) {
		struct live_interval *it, *prev;
		unsigned long position;

		current = pqueue_remove_top(unhandled);
		position = interval_start(current);

		list_for_each_entry_safe(it, prev, &active, interval_node) {
			if (interval_end(it) <= position) {
				/*
				 * Remove handled interval from active list.
				 */
				list_del(&it->interval_node);
				continue;
			}

			interval_expire_ranges_before(it, position);

			if (!interval_covers(it, position))
				list_move(&it->interval_node, &inactive);
		}

		list_for_each_entry_safe(it, prev, &inactive, interval_node) {
			if (interval_end(it) <= position) {
				/*
				 * Remove handled interval from inactive list.
				 */
				list_del(&it->interval_node);
				continue;
			}

			interval_expire_ranges_before(it, position);

			if (interval_covers(it, position))
				list_move(&it->interval_node, &active);
		}

		/*
		 * Don't allocate registers for fixed intervals.
		 */
		assert(!interval_has_fixed_reg(current));

		try_to_allocate_free_reg(cu, current, &active, &inactive, unhandled);

		if (current->reg == MACH_REG_UNASSIGNED)
			allocate_blocked_reg(cu, current, &active, &inactive, unhandled);

		if (current->reg != MACH_REG_UNASSIGNED)
			list_add(&current->interval_node, &active);
	}
	free(registers);

	for_each_variable(var, cu->var_infos) {
		struct live_interval *it = var->interval;

		while (it) {
			interval_restore_expired_ranges(it);
			it = it->next_child;
		}
	}

	pqueue_free(unhandled);

	cu->flags |= CU_FLAG_REGALLOC_DONE;

	return 0;
}
