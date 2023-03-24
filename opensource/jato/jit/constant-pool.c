#include "jit/constant-pool.h"

#include "jit/compilation-unit.h"

#include <stdlib.h>

static void insert_to_literal_pool(struct compilation_unit *cu, struct lp_entry *lp)
{
	if (cu->pool_head == NULL) {
		cu->pool_head = lp;
	} else {
		struct lp_entry *ptr;

		ptr = cu->pool_head;
		while (ptr->next != NULL)
			ptr = ptr->next;

		ptr->next = lp;
	}
}

/*
 * This functions is used by the constant pool implementation.
 * It returns the pointer to the node of immediate value.
 * If value is present previously in constant pool it will return pointer
 * to pre existing node otherwise it will make a new node and then return
 * the pointer
 */
struct lp_entry *alloc_literal_pool_entry(struct compilation_unit *cu, unsigned long value)
{
	struct lp_entry *lp;

	lp = search_literal_pool(cu->pool_head, value);
	if (lp)
		return lp;

	lp = malloc(sizeof *lp);
	if (!lp)
		return NULL;

	lp->index = cu->nr_entries_in_pool++;
	lp->value = value;

	insert_to_literal_pool(cu, lp);

	return lp;
}

/*
 * This functions searches the constant pool for the presence
 * of a given immediate value
 */
struct lp_entry *search_literal_pool(struct lp_entry *lp, unsigned long value)
{
	struct lp_entry *lp_ptr;

	lp_ptr = lp;
	while (lp_ptr) {
		if (lp_ptr->value == value)
			return lp_ptr;

		lp_ptr = lp_ptr->next;
	}

	return NULL;
}

/*
 * This function frees the memory occupied by constant pool
 */
void free_constant_pool(struct lp_entry *head)
{
	struct lp_entry *lp = head;

	while (lp != NULL) {
		head = head->next;
		free(lp);
		lp = head;
	}
}
