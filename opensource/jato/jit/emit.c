/*
 * Emits machine code.
 *
 * Copyright (C) 2007  Pekka Enberg
 *
 * This file is released under the GPL version 2. Please refer to the file
 * LICENSE for details.
 */

#include "arch/inline-cache.h"

#include "lib/buffer.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/object.h"
#include "vm/die.h"
#include "vm/vm.h"

#include "jit/compilation-unit.h"
#include "jit/basic-block.h"
#include "jit/compiler.h"
#include "jit/emit-code.h"
#include "jit/exception.h"
#include "jit/gdb.h"
#include "jit/instruction.h"
#include "jit/statement.h"
#include "jit/debug.h"
#include "jit/text.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

bool opt_debug_stack;

static void emit_monitorenter(struct compilation_unit *cu,
			      unsigned long frame_size)
{
	if (vm_method_is_static(cu->method))
		emit_lock(cu->objcode, cu->method->class->object);
	else
		emit_lock_this(cu->objcode, frame_size);
}

static void emit_monitorexit(struct compilation_unit *cu,
			     unsigned long frame_size)
{
	if (vm_method_is_static(cu->method))
		emit_unlock(cu->objcode, cu->method->class->object);
	else
		emit_unlock_this(cu->objcode, frame_size);
}

static void backpatch_tableswitch(struct tableswitch *table)
{
	int count;

	count = table->high - table->low + 1;

	for (int i = 0; i < count; i++) {
		int idx = bb_lookup_successor_index(table->src,
						    table->bb_lookup_table[i]);

		if (branch_needs_resolution_block(table->src, idx)) {
			table->lookup_table[i] =
				(void *)table->src->resolution_blocks[idx].addr;
		} else {
			table->lookup_table[i] =
				bb_native_ptr(table->bb_lookup_table[i]);
		}
	}
}

static void backpatch_lookupswitch(struct lookupswitch *table)
{
	for (unsigned int i = 0; i < table->count; i++) {
		int idx = bb_lookup_successor_index(table->src,
						    table->pairs[i].bb_target);

		if (branch_needs_resolution_block(table->src, idx)) {
			table->pairs[i].target =
				(void *)table->src->resolution_blocks[idx].addr;
		} else {
			table->pairs[i].target =
				bb_native_ptr(table->pairs[i].bb_target);
		}
	}
}

static void backpatch_tableswitch_targets(struct compilation_unit *cu)
{
	struct tableswitch *this;

	list_for_each_entry(this, &cu->tableswitch_list, list_node)
	{
		backpatch_tableswitch(this);
	}
}

static void backpatch_lookupswitch_targets(struct compilation_unit *cu)
{
	struct lookupswitch *this;

	list_for_each_entry(this, &cu->lookupswitch_list, list_node)
	{
		backpatch_lookupswitch(this);
	}
}

static void backpatch_branches(struct basic_block *bb, struct buffer *buf)
{
	struct insn *insn;

	for_each_insn(insn, &bb->insn_list) {
		if (insn->flags & INSN_FLAG_BACKPATCH_BRANCH) {
			struct basic_block *target_bb;

			target_bb = insn->operand.branch_target;

			backpatch_branch_target(buf, insn, target_bb->mach_offset);
		} else if (insn->flags & INSN_FLAG_BACKPATCH_RESOLUTION) {
			struct resolution_block *rb;

			rb = insn->operand.resolution_block;

			backpatch_branch_target(buf, insn, rb->mach_offset);
		}
	}
}

void emit_body(struct basic_block *bb, struct buffer *buf)
{
	struct insn *insn;

	bb->mach_offset = buffer_offset(buf);
	bb->is_emitted = true;

	for_each_insn(insn, &bb->insn_list) {
		emit_insn(buf, bb, insn);
	}

	if (opt_trace_machine_code)
		emit_nop(buf);
}

static void emit_resolution_blocks(struct basic_block *bb, struct buffer *buf)
{
	for (unsigned int i = 0; i < bb->nr_successors; i++) {
		struct resolution_block *block;
		unsigned long mach_offset;
		struct insn *insn;

		mach_offset = buffer_offset(buf);
		block = &bb->resolution_blocks[i];
		block->mach_offset = mach_offset;
		block->addr = (unsigned long) buffer_ptr(buf) + mach_offset;

		if (list_is_empty(&block->insns))
			continue;

		for_each_insn(insn, &block->insns) {
			emit_insn(buf, NULL, insn);
		}

		emit_insn(buf, NULL, jump_insn(bb->successors[i]));
	}
}

static void process_call_fixup_sites(struct compilation_unit *cu)
{
	struct fixup_site *site, *next;

	list_for_each_entry_safe(site, next, &cu->call_fixup_site_list, list_node) {
		site->mach_offset = site->relcall_insn->mach_offset;

		pthread_mutex_lock(&site->target->mutex);
		list_move(&site->list_node, &site->target->fixup_site_list);
		pthread_mutex_unlock(&site->target->mutex);
	}
}

int emit_machine_code(struct compilation_unit *cu)
{
	unsigned long frame_size;
	struct basic_block *bb;
	struct buffer *buf;
	int err = 0;
	void *ic_check = NULL;

	buf = alloc_exec_buffer();
	if (!buf)
		return warn("out of memory"), -ENOMEM;

	jit_text_lock();

	buf->buf = jit_text_ptr();
	cu->objcode = buf;

	frame_size = frame_locals_size(cu->stack_frame);

	cu->ic_entry_point = NULL;

	if (ic_supports_method(cu->method)) {
		cu->ic_entry_point = buffer_current(buf);
		ic_check = emit_ic_check(buf);
	}

	cu->entry_point = buffer_current(buf);

	emit_prolog(cu->objcode, cu->stack_frame, frame_size);

	if (vm_method_is_synchronized(cu->method))
		emit_monitorenter(cu, frame_size);

	if (opt_trace_invoke)
		emit_trace_invoke(cu->objcode, cu);

	for_each_basic_block(bb, &cu->bb_list)
		emit_body(bb, cu->objcode);

	emit_body(cu->exit_bb, cu->objcode);
	if (vm_method_is_synchronized(cu->method))
		emit_monitorexit(cu, frame_size);
	cu->exit_past_unlock_ptr = buffer_current(cu->objcode);
	emit_epilog(cu->objcode);

	emit_body(cu->unwind_bb, cu->objcode);
	if (vm_method_is_synchronized(cu->method))
		emit_monitorexit(cu, frame_size);
	cu->unwind_past_unlock_ptr = buffer_current(cu->objcode);
	emit_unwind(cu->objcode);

	for_each_basic_block(bb, &cu->bb_list) {
		emit_resolution_blocks(bb, cu->objcode);
	}

	for_each_basic_block(bb, &cu->bb_list) {
		backpatch_branches(bb, cu->objcode);
	}

	process_call_fixup_sites(cu);
	backpatch_tableswitch_targets(cu);
	backpatch_lookupswitch_targets(cu);
	build_exception_handlers_table(cu);

	cu->exit_bb_ptr = bb_native_ptr(cu->exit_bb);
	cu->unwind_bb_ptr = bb_native_ptr(cu->unwind_bb);

	if (ic_check) {
		emit_ic_miss_handler(buf, ic_check, cu->method);
	}

	jit_text_reserve(buffer_offset(cu->objcode));

	jit_text_unlock();

	gdb_register_method(cu->method);

	return err;
}

struct jit_trampoline *alloc_jit_trampoline(void)
{
	struct jit_trampoline *trampoline;

	trampoline = malloc(sizeof(*trampoline));
	if (!trampoline)
		return NULL;

	memset(trampoline, 0, sizeof(*trampoline));

	trampoline->objcode = alloc_exec_buffer();
	if (!trampoline->objcode)
		goto failed;

	INIT_LIST_HEAD(&trampoline->fixup_site_list);
	pthread_mutex_init(&trampoline->mutex, NULL);

	return trampoline;

  failed:
	free_jit_trampoline(trampoline);
	return NULL;
}

void free_jit_trampoline(struct jit_trampoline *trampoline)
{
	free_buffer(trampoline->objcode);
	free(trampoline);
}
