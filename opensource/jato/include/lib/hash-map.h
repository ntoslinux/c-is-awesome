#ifndef _JATO_HASH_MAP
#define _JATO_HASH_MAP

#include "lib/list.h"

struct hash_map_entry {
	unsigned long		hash;
	const void		*key;
	void			*value;
	struct list_head	list_node;
};

typedef unsigned long hash_fn(const void *);
typedef bool equals_fn(const void *, const void *);
typedef void final_process_fn(const void *);

struct key_operations {
	hash_fn *hash;
	equals_fn *equals;
	final_process_fn *final_process;
};

struct key_operations pointer_key;
struct key_operations string_key;

struct hash_map {
	struct key_operations key_ops;
	unsigned long size;

	double fill_factor;
	unsigned long resize_at;

	struct list_head *table;
	unsigned long table_size;
};

struct hash_map *alloc_hash_map(struct key_operations *key_ops);
struct hash_map *alloc_hash_map_with_size(unsigned long initial_size, struct key_operations *key_ops);
void free_hash_map(struct hash_map *map);
int hash_map_put(struct hash_map *map, const void *key, void *value);
int hash_map_get(struct hash_map *map, const void *key, void **value_p);
int hash_map_remove(struct hash_map *map, const void *key);
bool hash_map_contains(struct hash_map *map, const void *key);
int hash_map_size(struct hash_map *map);
bool hash_map_is_empty(struct hash_map *map);

unsigned long ptr_hash(const void *);
bool ptr_equals(const void *, const void *);

#define hash_map_for_each_entry(this, hashmap)				\
	for (unsigned long i__ = 0; i__ < (hashmap)->table_size; i__++)	\
		list_for_each_entry(this, &(hashmap)->table[i__], list_node)

#endif
