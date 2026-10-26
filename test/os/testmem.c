/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/testmem.h"

#include "cfg/conf.h"
#include "os/mem.h"

#include <errno.h>
#include <greatest/greatest.h>
#include <sys/mman.h>
#include <unistd.h>

TEST
josmemrequest_verify_eoverflow(void)
{
	ASSERT_EQ(josmemrequest(SIZE_MAX - JALLOC_MAXBLKSZ + 1), NULL);
	ASSERT_EQ(errno, EOVERFLOW);

	PASS();
}

TEST
josmemrequest_verify_einval(void)
{
	ASSERT_EQ(josmemrequest(0), NULL);
	ASSERT_EQ(errno, EINVAL);

	PASS();
}

TEST
josmemrequest_verify_memalign_p(size_t memsz)
{
	long pagesz_conf = sysconf(_SC_PAGESIZE);
	ASSERT_NEQm("sysconf(_SC_PAGESIZE) failed", pagesz_conf, -1);
	size_t pagesz = (size_t)pagesz_conf;

	void * mem = josmemrequest(memsz);

	ASSERT_EQ((uintptr_t)mem % JALLOC_MAXBLKSZ, 0);
	ASSERT_EQ((uintptr_t)mem % pagesz, 0);

	if (munmap(mem, memsz) == -1) FAILm("munmap failed");

	PASS();
}

static void
josmemrequest_verify_memalign(void)
{
	size_t pagesz = sysconf(_SC_PAGESIZE);

	/* little size test */
	RUN_TESTp(josmemrequest_verify_memalign_p, 1);

	/* page size test */
	RUN_TESTp(josmemrequest_verify_memalign_p, pagesz - 1);
	RUN_TESTp(josmemrequest_verify_memalign_p, pagesz);
	RUN_TESTp(josmemrequest_verify_memalign_p, pagesz + 1);

	/* maximum block size test */
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_MAXBLKSZ - 1);
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_MAXBLKSZ);
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_MAXBLKSZ + 1);

	/* chunk size test (main use case) */
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_CHUNKSZ - 1);
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_CHUNKSZ);
	RUN_TESTp(josmemrequest_verify_memalign_p, JALLOC_CHUNKSZ + 1);

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
