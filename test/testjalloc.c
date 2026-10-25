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

	ASSERT_NEQm(ptr1, NULL, "jalloc failed");
	ASSERT_NEQm(ptr48, NULL, "jalloc failed");
	ASSERT_NEQm(ptr1024, NULL, "jalloc failed");

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
	ASSERT_NEQm((void *)i8ptr, NULL, "jalloc failed");

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
	void *	     ptrs[ALLOCCNT];

	for (size_t i = 0; i < ALLOCCNT; i++) {
		ptrs[i] = jalloc(2 * JALLOC_MAXBLKSZ / ALLOCCNT);
		ASSERT_NEQm(ptrs[i], NULL, "jalloc failed");
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
	void *	     ptrs[ALLOCCNT];

	for (size_t i = 0; i < ALLOCCNT; i++) {
		ptrs[i] = jalloc(4 * JALLOC_MAXBLKSZ / ALLOCCNT);
		ASSERT_NEQm(ptrs[i], NULL, "jalloc failed");
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
jfree_verify_null(void)
{
	jfree(NULL);
	PASS();
}

TEST
jfree_verify_reusability(void)
{
	void * p1 = jalloc(64);
	ASSERT_NEQm(p1, NULL, "jalloc error");
	*((int32_t *)p1) = 0xDEADBEEF;

	jfree(p1);

	void * p2 = jalloc(64);
	ASSERT_NEQm(p2, NULL, "jalloc error");

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
	jfree_verify();
}
