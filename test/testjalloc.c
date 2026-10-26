/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "testjalloc.h"

#include "cfg/layout.h"

#include <errno.h>
#include <greatest/greatest.h>
#include <jalloc.h>
#include <stdint.h>

TEST
jalloc_verify_einval(void)
{
	ASSERT_EQ(jalloc(0), NULL);
	ASSERT_EQ(errno, EINVAL);

	PASS();
}

TEST
jalloc_verify_eoverflow(void)
{
	ASSERT_EQ(jalloc(SIZE_MAX - JALLOC_BLKHDRSZ + 1), NULL);
	ASSERT_EQ(errno, EOVERFLOW);

	PASS();
}

TEST
jalloc_verify_enomem(void)
{
	ASSERT_EQ(jalloc(JALLOC_MAXBLKSZ + 1), NULL);
	ASSERT_EQ(errno, ENOMEM);

	PASS();
}

/* this test assumes JALLOC_MAXBLKSZ >= 1024 + JALLOC_BLKHDRSZ */
TEST
jalloc_verify_alignment(void)
{
	void * ptr1    = jalloc(1);
	void * ptr48   = jalloc(48);
	void * ptr1024 = jalloc(1024);

	ASSERT_NEQm("jalloc failed", ptr1, NULL);
	ASSERT_NEQm("jalloc failed", ptr48, NULL);
	ASSERT_NEQm("jalloc failed", ptr1024, NULL);

	ASSERT_EQ((uintptr_t)ptr1 % JALLOC_ALIGN, 0);
	ASSERT_EQ((uintptr_t)ptr48 % JALLOC_ALIGN, 0);
	ASSERT_EQ((uintptr_t)ptr1024 % JALLOC_ALIGN, 0);

	jfree(ptr1);
	jfree(ptr48);
	jfree(ptr1024);

	PASS();
}

/*
 * this test assumes JALLOC_MAXBLKSZ > 4 * sizeof(uint8_t) + JALLOC_BLKHDRSZ
 *
 * (it's unlikely to be any other way, just for mathematical certainty) */
TEST
jalloc_verify_persistency(void)
{
	uint8_t * i8ptr = (uint8_t *)jalloc(4 * sizeof(uint8_t));
	ASSERT_NEQm("jalloc failed", (void *)i8ptr, NULL);

	*i8ptr	     = 0xDE;
	*(i8ptr + 1) = 0xAD;
	*(i8ptr + 2) = 0xBE;
	*(i8ptr + 3) = 0xEF;

	ASSERT_EQ(i8ptr[0], (uint8_t)0xDE);
	ASSERT_EQ(i8ptr[1], (uint8_t)0xAD);
	ASSERT_EQ(i8ptr[2], (uint8_t)0xBE);
	ASSERT_EQ(i8ptr[3], (uint8_t)0xEF);

	*i8ptr = 0x55;
	ASSERT_EQ(*i8ptr, 0x55);

	jfree(i8ptr);

	PASS();
}

/* first stress test */
TEST
jalloc_verify_stress(void)
{
	/* allocate 2 * JALLOC_MAXBLKSZ bytes (total) */
	size_t const ALLOCCNT = 128;
	void **	     ptrs = alloca(ALLOCCNT * sizeof(void *)); /* love MSVC very much! */

	for (size_t i = 0; i < ALLOCCNT; i++) {
		ptrs[i] = jalloc(2 * JALLOC_MAXBLKSZ / ALLOCCNT);
		ASSERT_NEQm("jalloc failed", ptrs[i], NULL);
	}

	/* trigger the coalescence */
	for (size_t i = ALLOCCNT; i > 0; i--) jfree(ptrs[i - 1]);

	/* is it possible to allocate big block? */
	void * big = jalloc(JALLOC_MAXBLKSZ - JALLOC_BLKHDRSZ);
	ASSERT_NEQ(big, NULL);
	jfree(big);

	PASS();
}

/* second stress test */
TEST
jalloc_verify_fragmentation(void)
{
	/* allocate 4 * JALLOC_MAXBLKSZ bytes (total) */
	size_t const ALLOCCNT = 256;
	void **	     ptrs = alloca(ALLOCCNT * sizeof(void *)); /* love MSVC very much! */

	for (size_t i = 0; i < ALLOCCNT; i++) {
		ptrs[i] = jalloc(4 * JALLOC_MAXBLKSZ / ALLOCCNT);
		ASSERT_NEQm("jalloc failed", ptrs[i], NULL);
	}

	/* the chessboard pattern: free odd blocks (0-indexation) */
	for (size_t i = 1; i < ALLOCCNT; i += 2) jfree(ptrs[i]);

	/* single block */
	void * big = jalloc(4096);
	ASSERT_NEQ(big, NULL);
	jfree(big);

	/* free the even blocks to prevent memory leaks */
	for (size_t i = 0; i < ALLOCCNT; i += 2) jfree(ptrs[i]);

	PASS();
}

static void
jalloc_verify(void)
{
	RUN_TEST(jalloc_verify_einval);
	RUN_TEST(jalloc_verify_eoverflow);
	RUN_TEST(jalloc_verify_enomem);
	RUN_TEST(jalloc_verify_alignment);
	RUN_TEST(jalloc_verify_persistency);
	RUN_TEST(jalloc_verify_stress);
	RUN_TEST(jalloc_verify_fragmentation);
}

TEST
jcalloc_verify_einval(void)
{
	ASSERT_EQ(jcalloc(0, sizeof(int)), NULL);
	ASSERT_EQ(errno, EINVAL);

	PASS();
}

TEST
jcalloc_verify_eoverflow(void)
{
	ASSERT_EQ(jcalloc(SIZE_MAX / 2, 4), NULL);
	ASSERT_EQ(errno, EOVERFLOW);

	PASS();
}

TEST
jcalloc_verify_zeroinit(void)
{
	size_t nmemb = 112;
	int *  arr   = (int *)jcalloc(nmemb, sizeof(int));
	ASSERT_NEQm("jcalloc failed", (void *)arr, NULL);

	for (size_t i = 0; i < nmemb; i++) ASSERT_EQ(arr[i], 0);
	jfree(arr);

	PASS();
}

static void
jcalloc_verify(void)
{
	RUN_TEST(jcalloc_verify_einval);
	RUN_TEST(jcalloc_verify_eoverflow);
	RUN_TEST(jcalloc_verify_zeroinit);
}

TEST
jrealloc_verify_jalloc(void)
{
	size_t const TESTSZ = 112;

	/* EINVAL for zero size verifying */
	errno		    = 0;
	void * rezero	    = jrealloc(NULL, 0);
	int    rezero_errno = errno;

	errno		   = 0;
	void * jzero	   = jalloc(0);
	int    jzero_errno = errno;

	ASSERT_EQ(rezero, NULL);
	ASSERT_EQ(jzero, NULL);
	ASSERT_EQ(jzero_errno, rezero_errno);

	/* EOVERFLOW verifying */
	errno			= 0;
	void * reoverflow	= jrealloc(NULL, SIZE_MAX);
	int    reoverflow_errno = errno;

	errno		       = 0;
	void * joverflow       = jalloc(SIZE_MAX);
	int    joverflow_errno = errno;

	ASSERT_EQ(reoverflow, NULL);
	ASSERT_EQ(joverflow, NULL);
	ASSERT_EQ(joverflow_errno, reoverflow_errno);

	/* ENOMEM verifying */
	errno		     = 0;
	void * renomem	     = jrealloc(NULL, JALLOC_MAXBLKSZ + 1);
	int    renomem_errno = errno;

	errno		    = 0;
	void * jnomem	    = jalloc(JALLOC_MAXBLKSZ + 1);
	int    jnomem_errno = errno;

	ASSERT_EQ(renomem, NULL);
	ASSERT_EQ(jnomem, NULL);
	ASSERT_EQ(jnomem_errno, renomem_errno);

	/* successful allocation verifying */
	void * rep = jrealloc(NULL, TESTSZ);
	void * jp  = jalloc(TESTSZ);

	ASSERT_NEQm("jrealloc(NULL, sz) failed", rep, NULL);
	ASSERT_NEQm("jalloc failed", jp, NULL);

	/* alignment verifying */
	ASSERT_EQ((uintptr_t)rep % JALLOC_ALIGN, (uintptr_t)jp % JALLOC_ALIGN);

	/* equal block order verifying */
	uint8_t reorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(rep));
	uint8_t jorder	= JALLOC_GET_ORDER(JALLOC_BLKHDR(jp));
	ASSERT_EQ(reorder, jorder);

	jfree(rep);
	jfree(jp);

	PASS();
}

TEST
jrealloc_verify_jfree(void)
{
	void * p1 = jalloc(48);
	ASSERT_NEQm("jalloc failed", p1, NULL);
	void * hdr1 = JALLOC_BLKHDR(p1);
	jfree(p1);

	void * p2 = jalloc(48);
	ASSERT_NEQm("jalloc failed", p2, NULL);
	void * hdr2 = JALLOC_BLKHDR(p2);
	ASSERT_EQ(jrealloc(p2, 0), NULL);

	ASSERT_EQ(JALLOC_GET_FREE(hdr1), JALLOC_GET_FREE(hdr2));

	void * p3 = jalloc(48);

	/* p3 will get the same address as p2 and p1 */
	ASSERT_EQ(p3, p2);
	ASSERT_EQ(p3, p1);

	jfree(p3);

	PASS();
}

TEST
jrealloc_verify_reduce(void)
{
	size_t const OLDSZ = 112;
	size_t const NEWSZ = 16;

	uint8_t * ptr = (uint8_t *)jalloc(OLDSZ);
	ASSERT_NEQm("jalloc failed", (void *)ptr, NULL);
	uint8_t oldorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(ptr));

	for (size_t i = 0; i < NEWSZ; i++) ptr[i] = (uint8_t)i;

	uint8_t * reptr = (uint8_t *)jrealloc(ptr, NEWSZ);
	ASSERT_NEQm("jrealloc failed", (void *)reptr, NULL);
	uint8_t neworder = JALLOC_GET_ORDER(JALLOC_BLKHDR(reptr));

	ASSERT_EQ(ptr, reptr); /* due to buddy logic, adresses will be the same */
	ASSERT(
	    memcmp(ptr, reptr, NEWSZ) == 0); /* everything past NEWSZ will be lost, and i don't care about UAF here */
	ASSERT_LT(neworder, oldorder);

	jfree(reptr);

	PASS();
}

TEST
jrealloc_verify_same_order(void)
{
	size_t const OLDSZ = 112;
	size_t const NEWSZ = 96;

	uint8_t * ptr = (uint8_t *)jalloc(OLDSZ);
	ASSERT_NEQm("jalloc failed", (void *)ptr, NULL);
	uint8_t oldorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(ptr));

	for (size_t i = 0; i < NEWSZ; i++) ptr[i] = (uint8_t)i;

	uint8_t * reptr = (uint8_t *)jrealloc(ptr, NEWSZ);
	ASSERT_NEQm("jrealloc failed", (void *)reptr, NULL);
	uint8_t neworder = JALLOC_GET_ORDER(JALLOC_BLKHDR(reptr));

	ASSERT_EQ(ptr, reptr); /* adresses will be the same, because realloc does nothing in this case */
	ASSERT(memcmp(ptr, reptr, NEWSZ) == 0); /* don't care about UAF */
	ASSERT_EQ(neworder, oldorder);

	jfree(reptr);

	PASS();
}

TEST
jrealloc_verify_expand(void)
{
	size_t const OLDSZ = 48;
	size_t const NEWSZ = 112;

	uint8_t * ptr = (uint8_t *)jalloc(OLDSZ);
	ASSERT_NEQm("jalloc failed", (void *)ptr, NULL);
	uint8_t oldorder = JALLOC_GET_ORDER(JALLOC_BLKHDR(ptr));

	for (size_t i = 0; i < OLDSZ; i++) ptr[i] = (uint8_t)i;

	uint8_t * reptr = (uint8_t *)jrealloc(ptr, NEWSZ);
	ASSERT_NEQm("realloc failed", (void *)reptr, NULL);
	uint8_t neworder = JALLOC_GET_ORDER(JALLOC_BLKHDR(reptr));

	for (size_t i = 0; i < OLDSZ; i++) ASSERT_EQ(reptr[i], (uint8_t)i);
	ASSERT_GT(neworder, oldorder);

	jfree(reptr);

	PASS();
}

TEST
jrealloc_verify_persistency(void)
{
	void * ptr = jalloc(48);
	ASSERT_NEQm("jalloc failed", ptr, NULL);

	void * newptr = jrealloc(ptr, SIZE_MAX);
	ASSERT_EQm("unexpected jrealloc behaviour", newptr, NULL);

	/* ptr must stay be valid and point to allocated memory area */
	ASSERT_EQ(JALLOC_GET_FREE(JALLOC_BLKHDR(ptr)), 0x00);

	PASS();
}

static void
jrealloc_verify(void)
{
	RUN_TEST(jrealloc_verify_jalloc);
	RUN_TEST(jrealloc_verify_jfree);
	RUN_TEST(jrealloc_verify_reduce);
	RUN_TEST(jrealloc_verify_same_order);
	RUN_TEST(jrealloc_verify_expand);
	RUN_TEST(jrealloc_verify_persistency);
}

TEST
jfree_verify_null(void)
{
	jfree(NULL);
	PASS();
}

TEST
jfree_verify_reusability(void)
{
	void * p1 = jalloc(64);
	ASSERT_NEQm("jalloc failed", p1, NULL);
	*((int32_t *)p1) = 0xDEADBEEF;

	jfree(p1);

	void * p2 = jalloc(64);
	ASSERT_NEQm("jalloc failed", p2, NULL);

	ASSERT_EQ(p1, p2); /* p2 will get the same address as p1 */

	jfree(p2);

	PASS();
}

static void
jfree_verify(void)
{
	RUN_TEST(jfree_verify_null);
	RUN_TEST(jfree_verify_reusability);
}

SUITE(jalloc_pub)
{
	jalloc_verify();
	jcalloc_verify();
	jrealloc_verify();
	jfree_verify();
}
