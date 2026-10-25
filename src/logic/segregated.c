/**
 * @file logic/segregated.c
 * @brief Segregated free list related data structures & algorithms functions definitions.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/segregated.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "os/mem.h"

/**
 * @brief Free lists.
 * @internal
 *
 * @note Array will be filled with zeros by default due to its location in the .bss section
 */
static void * jfreelists_[JALLOC_ORDERCNT];

int
jmemrequest(void)
{
	void * chunkp = josmemrequest(JALLOC_CHUNKSZ);
	if (!chunkp) return -1; /* no need to cover this branch via unit-tests */

	for (size_t offset = 0; offset < JALLOC_CHUNKSZ; offset += JALLOC_MAXBLKSZ) {
		void * blkhdr = (void *)((jalloc_byte_t *)chunkp + offset);

		JALLOC_SET_BLKHDR(blkhdr, JALLOC_MAXORDER, 0x01);
		jfreelist_pushblk(JALLOC_PAYLOAD(blkhdr));
	}

	return 0;
}

/* this function will not be tested due to its simplicity and high module cohesion */
void *
jfreelist_peek(uint8_t order)
{ return jfreelists_[order - JALLOC_MINORDER]; }

void
jfreelist_pushblk(void * blkp)
{
	uint8_t relorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp)) - JALLOC_MINORDER;
	void *	head	 = jfreelists_[relorder];

	JALLOC_FREELISTPREV(blkp) = NULL;
	JALLOC_FREELISTNEXT(blkp) = head;

	if (head) JALLOC_FREELISTPREV(head) = blkp;

	jfreelists_[relorder] = blkp;
}

void
jfreelist_removeblk(void * blkp)
{
	uint8_t relorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp)) - JALLOC_MINORDER;
	void *	prevblkp = JALLOC_FREELISTPREV(blkp);
	void *	nextblkp = JALLOC_FREELISTNEXT(blkp);

	if (prevblkp) JALLOC_FREELISTNEXT(prevblkp) = nextblkp;
	if (nextblkp) JALLOC_FREELISTPREV(nextblkp) = prevblkp;

	if (!prevblkp) jfreelists_[relorder] = nextblkp;
}

#ifdef JALLOC_UNITTESTING
void
jfreelists_reset(void)
{
	for (size_t i = 0; i < JALLOC_ORDERCNT; i++) jfreelists_[i] = NULL;
}
#endif /* JALLOC_UNITTESTING */
