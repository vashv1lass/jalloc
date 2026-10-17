#ifndef JALLOC_LOGIC_SEGREGATED_H_
#define JALLOC_LOGIC_SEGREGATED_H_

#include "cfg/conf.h"

#include <stdint.h>

static inline uint8_t
jblkorder(size_t blksz)
{
	/* order = max(0, ceil(log2(x))) */
	uint8_t res = 0;

	if (blksz <= JALLOC_MINBLKSZ) return JALLOC_MINORDER; /* max(0, ...) */
	blksz--;					      /* ceil(log2(x)) == floor(log2(x - 1)) + 1 */

	/* __builtin_clzll(x) */
#if SIZE_MAX > 0xFFFFFFFF
	if (blksz >= 0x100000000ULL) res += 32, blksz >>= 32;
#endif
	if (blksz >= 0x10000) res += 16, blksz >>= 16;
	if (blksz >= 0x100) res += 8, blksz >>= 8;
	if (blksz >= 0x10) res += 4, blksz >>= 4;
	if (blksz >= 0x4) res += 2, blksz >>= 2;
	if (blksz >= 0x2) res++;

	return res + 1;
}

int
jmemrequest(void);

void
jfreelist_pushblk(void * blkp);

void
jfreelist_removeblk(void * blkp);

void *
jfreelist_peek(uint8_t order);

#endif /* JALLOC_LOGIC_FREELIST_H_ */
