#ifndef JATO__PPC_CMPXCHG_H
#define JATO__PPC_CMPXCHG_H

#include <stdint.h>
#include <assert.h>

static inline uint32_t cmpxchg_32(uint32_t *p, uint32_t old, uint32_t new)
{
	return __sync_val_compare_and_swap(p, old, new);
}

static inline uint64_t cmpxchg_64(uint64_t *p, uint64_t old, uint64_t new)
{
	assert(!"not implemented");
}

static inline void *cmpxchg_ptr(void *p, void *old, void *new)
{
	return (void *) cmpxchg_32((uint32_t *) p, (uint32_t) old, (uint32_t) new);
}

#endif /* JATO__PPC_CMPXCHG_H */
