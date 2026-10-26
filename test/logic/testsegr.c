/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/testsegr.h"

#include "cfg/layout.h"
#include "logic/segregated.h"
#include "logic/testutils.h"
#include "os/mem.h"

#include <greatest/greatest.h>
#include <limits.h>
#include <math.h>

#define JALLOC_TEST_MAX(x, y) ((x) > (y) ? (x) : (y))

TEST
jblkorder_verify_formula(size_t blksz)
{
	/* don't care about double -> size_t conversion loss lmao */
	ASSERT_EQ(jblkorder(blksz), JALLOC_TEST_MAX(JALLOC_MINORDER, (uint8_t)ceil(log2(blksz))));
	PASS();
}

static void
jblkorder_verify(void)
{
	/* min allowed size (domain) */
	RUN_TESTp(jblkorder_verify_formula, 1);

	/* min size border */
	RUN_TESTp(jblkorder_verify_formula, JALLOC_MINBLKSZ - 1);
	RUN_TESTp(jblkorder_verify_formula, JALLOC_MINBLKSZ);
	RUN_TESTp(jblkorder_verify_formula, JALLOC_MINBLKSZ + 1);

	/* 0x2 */
	RUN_TESTp(jblkorder_verify_formula, 2);
	RUN_TESTp(jblkorder_verify_formula, 3);

	/* 0x4 */
	RUN_TESTp(jblkorder_verify_formula, 4);
	RUN_TESTp(jblkorder_verify_formula, 5);

	/* 0x10 */
	RUN_TESTp(jblkorder_verify_formula, 15);
	RUN_TESTp(jblkorder_verify_formula, 16);
	RUN_TESTp(jblkorder_verify_formula, 17);

	/* 0x100 */
	RUN_TESTp(jblkorder_verify_formula, 255);
	RUN_TESTp(jblkorder_verify_formula, 256);
	RUN_TESTp(jblkorder_verify_formula, 257);

	/* 0x10000 */
	RUN_TESTp(jblkorder_verify_formula, 65535);
	RUN_TESTp(jblkorder_verify_formula, 65536);
	RUN_TESTp(jblkorder_verify_formula, 65537);

#if SIZE_MAX > 0xFFFFFFFF
	/* 0x100000000 */
	RUN_TESTp(jblkorder_verify_formula, 4294967295ULL);
	RUN_TESTp(jblkorder_verify_formula, 4294967296ULL);
	RUN_TESTp(jblkorder_verify_formula, 4294967297ULL);
#endif

	/* extreme value */
	RUN_TESTp(jblkorder_verify_formula, SIZE_MAX);
}

TEST
jmemrequest_verify(void)
{
	jfreelists_reset();

	/* memory request */
	ASSERT_EQ(jmemrequest(), 0);
	size_t sz = jfreelistsz(JALLOC_MAXORDER);

	/* correct pushed blocks count */
	ASSERT_EQ(sz, JALLOC_CHUNKSZ / JALLOC_MAXBLKSZ);

	/* block structure check */
	void * firstblkp = jfreelist_peek(JALLOC_MAXORDER);
	void * curblkp = firstblkp;
	for (size_t i = 0; i < sz; i++) {
		void * curblkhdr = JALLOC_BLKHDR(curblkp);

		ASSERT_EQ(JALLOC_GET_ORDER(curblkhdr), JALLOC_MAXORDER);
		ASSERT_EQ(JALLOC_GET_FREE(curblkhdr), 0x01);

		curblkp = JALLOC_FREELISTNEXT(curblkp);

		/* posix and windows unmapping is very different */
#if _POSIX_C_SOURCE >= 200809L
		/* unmap every block to avoid memory leaks */
		if (josmemreturn(curblkhdr, JALLOC_MAXBLKSZ) == -1) FAILm("josmemreturn failed");
#endif /* _POSIX_C_SOURCE >= 200809L */
	}

#ifdef _WIN32
	/* free the whole chunk only once from the arbitrary location */
	if (josmemreturn(firstblkp, 0) == -1) FAILm("josmemreturn failed");
#endif

	PASS();
}

TEST
jfreelist_pushblk_verify(void)
{
	jfreelists_reset();

	/* allocate test blocks on stack (do not forget about the alignment) */
	/* 2 blocks are quite enough (3 and more blocks will work similarly) */
	_Alignas(JALLOC_ALIGN) char blkbufs[2][JALLOC_MINBLKSZ];
	void *			    blkhdrs[2];
	void *			    blkptrs[2];

	for (size_t i = 0; i < 2; i++) {
		/* init the blocks */
		blkhdrs[i] = (void *)blkbufs[i];
		blkptrs[i] = JALLOC_PAYLOAD(blkhdrs[i]);
		JALLOC_SET_BLKHDR(blkhdrs[i], JALLOC_MINORDER, 0x01);

		jfreelist_pushblk(blkptrs[i]);
		if (i == 0) {
			/* add the first block into the list */
			ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 1);

			ASSERT_EQ(jfreelist_peek(JALLOC_MINORDER), blkptrs[i]);
			ASSERT_EQ(JALLOC_FREELISTPREV(blkptrs[i]), NULL);
			ASSERT_EQ(JALLOC_FREELISTNEXT(blkptrs[i]), NULL);
		} else {
			/* add the block to the head of the list */
			ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 2);

			ASSERT_EQ(jfreelist_peek(JALLOC_MINORDER), blkptrs[i]);
			ASSERT_EQ(JALLOC_FREELISTPREV(blkptrs[i]), NULL);
			ASSERT_EQ(JALLOC_FREELISTNEXT(blkptrs[i]), blkptrs[i - 1]);

			/* nextblock->prev check */
			ASSERT_EQ(JALLOC_FREELISTPREV(blkptrs[i - 1]), blkptrs[i]);
		}
	}

	PASS();
}

TEST
jfreelist_removeblk_verify(void)
{
	jfreelists_reset();

	/* allocate test blocks on stack (remember about alignment) */
	/* 4 blocks are quite enough to cover all the cases */
	_Alignas(JALLOC_ALIGN) char blkbufs[4][JALLOC_MINBLKSZ];
	void *			    blkhdrs[4];
	void *			    blkptrs[4];

	for (size_t i = 0; i < 4; i++) {
		blkhdrs[i] = (void *)blkbufs[i];
		blkptrs[i] = JALLOC_PAYLOAD(blkhdrs[i]);

		JALLOC_SET_BLKHDR(blkhdrs[i], JALLOC_MINORDER, 0x01);
		jfreelist_pushblk(blkptrs[i]);
	}

	/*
	 * free list structure at this point:
	 * NULL <-> blkptr[3] <-> blkptr[2] <-> blkptr[1] <-> blkptr[0] <-> NULL
	 */

	/* general remove */
	void * curremoved = blkptrs[2];

	jfreelist_removeblk(curremoved);

	/* NULL <-> blkptrs[3] <-> blkptrs[1] <-> blkptrs[0] <-> NULL */
	ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 3);
	ASSERT_EQ(JALLOC_FREELISTNEXT(JALLOC_FREELISTPREV(curremoved)), blkptrs[1]);
	ASSERT_EQ(JALLOC_FREELISTPREV(JALLOC_FREELISTNEXT(curremoved)), blkptrs[3]);

	/* head remove */
	curremoved = blkptrs[3];

	jfreelist_removeblk(curremoved);

	/* NULL <-> blkptrs[1] <-> blkptrs[0] <-> NULL */
	ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 2);
	ASSERT_EQ(JALLOC_FREELISTNEXT(jfreelist_peek(JALLOC_MINORDER)), blkptrs[0]); /* new head */
	ASSERT_EQ(JALLOC_FREELISTPREV(JALLOC_FREELISTNEXT(curremoved)), NULL);	     /* blkptrs[1]->prev */

	/* tail remove */
	curremoved = blkptrs[0];

	jfreelist_removeblk(curremoved);

	/* NULL <-> blkptrs[1] <-> NULL */
	ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 1);
	ASSERT_EQ(JALLOC_FREELISTNEXT(JALLOC_FREELISTPREV(curremoved)), NULL); /* blkptrs[1]->next */
	ASSERT_EQ(JALLOC_FREELISTPREV(jfreelist_peek(JALLOC_MINORDER)), NULL); /* new tail */

	/* single remove */
	curremoved = blkptrs[1];

	jfreelist_removeblk(curremoved);

	ASSERT_EQ(jfreelistsz(JALLOC_MINORDER), 0);

	PASS();
}

SUITE(jsegr)
{
	jblkorder_verify();
	RUN_TEST(jmemrequest_verify);
	RUN_TEST(jfreelist_pushblk_verify);
	RUN_TEST(jfreelist_removeblk_verify);
}
