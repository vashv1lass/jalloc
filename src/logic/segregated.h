/**
 * @file logic/segregated.h
 * @brief Segregated free list related data structures & algorithms.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_LOGIC_SEGREGATED_H_
#define JALLOC_LOGIC_SEGREGATED_H_

#include "cfg/conf.h"

#include <stdint.h>

/**
 * @defgroup segregated Segregated free list related data structures & algorithms.
 * @internal
 *
 * @{
 */

/**
 * @brief Get the block order by its size.
 *
 * @param blksz The size of the block.
 *
 * @return The order of a block.
 *
 * @note The order is counted by the formula of max(0, ceil(log2(\p blksz)))
 * @note The formula's value is calulated via some kind of maths & bit magic, see the comments in function to figure it
 * out.
 */
static inline uint8_t
jblkorder(size_t blksz)
{
	/* order = max(JALLOC_MINORDER, ceil(log2(blksz))) */
	uint8_t res = 0;

	if (blksz <= JALLOC_MINBLKSZ) return JALLOC_MINORDER; /* max(JALLOC_MINORDER, ...) */

	blksz--; /* ceil(log2(x)) == floor(log2(x - 1)) + 1 */

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

/**
 * @brief Requests JALLOC_CHUNKSZ bytes of memory from OS, creates free blocks from it and pushes them to the free list.
 *
 * @return 0 on success.
 * @return -1 on failure (errno is set).
 */
int
jmemrequest(void);

/**
 * @brief Pushes the block to the appropriate free list according to its order.
 *
 * @param blkp The pointer to the block to push (not NULL).
 *
 * @return None.
 */
void
jfreelist_pushblk(void * blkp);

/**
 * @brief Removes the block from the appropriate free list according to its order.
 *
 * @param blkp The pointer to the block to remove (not NULL).
 *
 * @return None.
 */
void
jfreelist_removeblk(void * blkp);

/**
 * @brief Gets the head of the free list of blocks with the certain order.
 *
 * @param order The order of the free list to return the head from.
 *
 * @return Pointer to the head of the free list of blocks with the order of \p order on success.
 * @return NULL if the free list of blocks with the order of \p order is empty.
 */
void *
jfreelist_peek(uint8_t order);

/** @} */

#endif /* JALLOC_LOGIC_FREELIST_H_ */
