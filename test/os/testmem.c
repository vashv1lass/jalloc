/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/testmem.h"

#include "cfg/conf.h"
#include "os/mem.h"
#include "os/testutils.h"

#include <errno.h>
#include <greatest/greatest.h>

TEST
josmemrequest_verify_eoverflow(void)
{
	size_t pagesz = getpagesz();
	ASSERT_NEQm("getpagesz failed", pagesz, SIZE_MAX);
	size_t memsz = JALLOC_ALIGNUP(SIZE_MAX - pagesz - 1, pagesz);

	errno = 0;
	ASSERT_EQ(josmemrequest(memsz), NULL);
	ASSERT_EQ(errno, EOVERFLOW);

	PASS();
}

TEST
josmemrequest_verify_einval(void)
{
	errno = 0;
	ASSERT_EQ(josmemrequest(0), NULL);
	ASSERT_EQ(errno, EINVAL);

	size_t pagesz = getpagesz();
	ASSERT_NEQm("getpagesz failed", pagesz, SIZE_MAX);

	errno = 0;
	ASSERT_EQ(josmemrequest(pagesz - 1), NULL);
	ASSERT_EQ(errno, EINVAL);

	PASS();
}

TEST
josmemrequest_verify_memalign_p(size_t memsz)
{
	size_t pagesz = getpagesz();
	ASSERT_NEQm("getpagesz failed", pagesz, SIZE_MAX);

	void * mem = josmemrequest(memsz);

	ASSERT_EQ((uintptr_t)mem % JALLOC_MAXBLKSZ, 0);
	ASSERT_EQ((uintptr_t)mem % pagesz, 0);

	if (josmemreturn(mem, memsz) == -1) FAILm("josmemreturn failed");

	PASS();
}

static void
josmemrequest_verify_memalign(void)
{
	size_t pagesz = getpagesz();

	/* page size test */
	if (pagesz != SIZE_MAX) {
		/* can't assert page size there (its not a test), so i just do that */
		RUN_TESTp(josmemrequest_verify_memalign_p, pagesz);
	}

	/* maximum block size test (assumes that JALLOC_MAXBLKSZ % pagesz == 0) */
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_MAXBLKSZ);

	/* chunk size test (main use case) */
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_CHUNKSZ);

	/* extreme size is environment-specific, so it wouldn't be tested */
}

static void
josmemrequest_verify(void)
{
	RUN_TEST(josmemrequest_verify_eoverflow);
	RUN_TEST(josmemrequest_verify_einval);
	josmemrequest_verify_memalign();
}

SUITE(josmem) { josmemrequest_verify(); }
