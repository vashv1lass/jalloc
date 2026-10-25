/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/testbuddy.h"

#include "cfg/layout.h"
#include "logic/buddy.h"
#include "logic/segregated.h"
#include "logic/testutils.h"

#include <greatest/greatest.h>

#define JALLOC_MIDORDER ((JALLOC_MAXORDER + JALLOC_MINORDER) / 2)
#define JALLOC_MIDBLKSZ (1 << JALLOC_MIDORDER)

TEST
jsplit_verify(void)
{
	jfreelists_reset();

	/* allocated block of some valid order (e.g. average) */
	_Alignas(JALLOC_MIDBLKSZ) char blkbuf[JALLOC_MIDBLKSZ];
	void *			       blkhdr = (void *)blkbuf;
	void *			       blkp   = JALLOC_PAYLOAD(blkhdr);
	JALLOC_SET_BLKHDR(blkhdr, JALLOC_MIDORDER, 0x00);

	jsplit(blkp, JALLOC_MIDORDER, JALLOC_MINORDER);

	/* all new formed buddies are free */
	for (uint8_t i = JALLOC_MINORDER; i < JALLOC_MIDORDER; i++) {
		ASSERT_EQ(jfreelistsz(i), 1);

		void * expectedrhs = (void *)((jalloc_byte_t *)blkp + ((ptrdiff_t)1 << i));
		ASSERT_EQ(jfreelist_peek(i), expectedrhs);

		void * rhshdr = JALLOC_BLKHDR(expectedrhs);
		ASSERT_EQ(JALLOC_GET_ORDER(rhshdr), i);
		ASSERT_EQ(JALLOC_GET_FREE(rhshdr), 0x01);
	}

	/* blkp is still allocated and has the order of JALLOC_MINORDER */
	ASSERT_EQ(JALLOC_GET_ORDER(blkhdr), JALLOC_MINORDER);
	ASSERT_EQ(JALLOC_GET_FREE(blkhdr), 0x00);

	PASS();
}

TEST
jcoalesce_verify(void)
{
	jfreelists_reset();

	/* split blocks */
	_Alignas(JALLOC_MIDBLKSZ) char blkbuf[JALLOC_MIDBLKSZ];
	void *			       blkhdr = (void *)blkbuf;
	void *			       blkp   = JALLOC_PAYLOAD(blkhdr);
	JALLOC_SET_BLKHDR(blkhdr, JALLOC_MIDORDER, 0x00);
	jsplit(blkp, JALLOC_MIDORDER, JALLOC_MINORDER);

	/* free imitation */
	JALLOC_SET_BLKHDR(blkhdr, JALLOC_GET_ORDER(blkhdr), 0x01);

	/* coalesced block is the same as the initial block */
	void * coalescedhdr = jcoalesce(blkhdr);
	ASSERT_EQ(coalescedhdr, blkhdr);
	ASSERT_EQ(JALLOC_GET_ORDER(coalescedhdr), JALLOC_GET_ORDER(blkhdr));
	ASSERT_EQ(JALLOC_GET_FREE(coalescedhdr), 0x01);

	for (uint8_t i = JALLOC_MINORDER; i < JALLOC_MIDORDER; i++) ASSERT_EQ(jfreelistsz(i), 0);

	PASS();
}

SUITE(jbuddy)
{
	RUN_TEST(jsplit_verify);
	RUN_TEST(jcoalesce_verify);
}
