/*
 * Copyright (c) 2009 Tomasz Grabiec
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
#include "jit/cu-mapping.h"

#include "vm/alloc.h"
#include "lib/radix-tree.h"
#include "lib/buffer.h"
#include "vm/die.h"

#include <pthread.h>

/*
 * A radix tree is used to associate native method addresses with
 * compilation unit. Only method entry address is stored in the tree
 * structure, so that inserting and removing compilation unit mapping
 * is fast.
 */
static struct radix_tree *cu_map;
static pthread_rwlock_t cu_map_rwlock = PTHREAD_RWLOCK_INITIALIZER;

#define BITS_PER_LEVEL 6

void init_cu_mapping(void)
{
	int key_bits = sizeof(unsigned long) * 8;

	cu_map = alloc_radix_tree(BITS_PER_LEVEL, key_bits);
	if (!cu_map)
		die("out of memory");
}

int add_cu_mapping(unsigned long addr, struct compilation_unit *cu)
{
	int result;

	pthread_rwlock_wrlock(&cu_map_rwlock);
	result = radix_tree_insert(cu_map, addr, cu);
	pthread_rwlock_unlock(&cu_map_rwlock);

	return result;
}

void remove_cu_mapping(unsigned long addr)
{
	pthread_rwlock_wrlock(&cu_map_rwlock);
	radix_tree_remove(cu_map, addr);
	pthread_rwlock_unlock(&cu_map_rwlock);
}

struct compilation_unit *jit_lookup_cu(unsigned long addr)
{
	struct compilation_unit *cu;

	pthread_rwlock_rdlock(&cu_map_rwlock);
	cu = radix_tree_lookup_prev(cu_map, addr);
	pthread_rwlock_unlock(&cu_map_rwlock);

	return cu;
}
