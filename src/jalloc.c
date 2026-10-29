/**
 * @file jalloc.c
 * @brief Public jalloc allocator interface function definitions.
 *
 * @author Maksim Vashkevich
 * @date 10/16/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "jalloc.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "logic/buddy.h"
#include "logic/segregated.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *
jalloc(size_t sz)
{
	uint8_t targorder, factorder;
	void *	blkp = NULL;

	if (sz == 0) {
		errno = EINVAL;
		return NULL;
	}

	if (sz > SIZE_MAX - JALLOC_BLKHDRSZ) {
		errno = EOVERFLOW;
		return NULL;
	}

	if ((targorder = jblkorder(sz + JALLOC_BLKHDRSZ)) > JALLOC_MAXORDER) {
		errno = ENOMEM;
		return NULL;
	}

	while (1) {
		for (factorder = targorder; factorder <= JALLOC_MAXORDER; factorder++) {
			blkp = jfreelist_peek(factorder);
			if (blkp) {
				jfreelist_removeblk(blkp);
				break;
			}
		}

		if (blkp) break;

		if (jmemrequest() == -1) return NULL;
	}

	jsplit(blkp, factorder, targorder);
	JALLOC_SET_BLKHDR(JALLOC_BLKHDR(blkp), targorder, 0x00);

	return blkp;
}

void *
jcalloc(size_t nmemb, size_t sz)
{
	if (nmemb == 0 || sz == 0) {
		errno = EINVAL;
		return NULL;
	}

	if (sz > SIZE_MAX / nmemb) {
		errno = EOVERFLOW;
		return NULL;
	}

	size_t totalsz = nmemb * sz;
	void * ptr     = jalloc(totalsz);
	if (ptr != NULL) memset(ptr, 0, totalsz);

	return ptr;
}

void *
jrealloc(void * ptr, size_t sz)
{
	if (ptr == NULL) return jalloc(sz);
	if (sz == 0) {
		jfree(ptr);
		return NULL;
	}

	if (sz > SIZE_MAX - JALLOC_BLKHDRSZ) {
		errno = EOVERFLOW;
		return NULL;
	}

	void *	blkhdr	 = JALLOC_BLKHDR(ptr);
	uint8_t oldorder = JALLOC_GET_ORDER(blkhdr);
	size_t	oldcap	 = (1 << oldorder) - JALLOC_BLKHDRSZ; /* bytes count to copy if necessary */

	uint8_t reqorder = jblkorder(sz + JALLOC_BLKHDRSZ);
	if (reqorder > JALLOC_MAXORDER) {
		errno = ENOMEM;
		return NULL;
	}

	/* split to avoid internal fragmentation */
	if (reqorder < oldorder) {
		jsplit(ptr, oldorder, reqorder);
		JALLOC_SET_BLKHDR(blkhdr, reqorder, 0x00);
		return ptr;
	}

	/* do nothing */
	if (reqorder == oldorder) return ptr;

	/* allocate new, copy, free old, return */
	void * newptr = jalloc(sz);
	if (newptr == NULL) return NULL;

	memcpy(newptr, ptr, oldcap);
	jfree(ptr);

	return newptr;
}

void
jfree(void * ptr)
{
	if (ptr == NULL) return;

	void *	blkhdr = JALLOC_BLKHDR(ptr);
	uint8_t order  = JALLOC_GET_ORDER(blkhdr);

	if (JALLOC_GET_FREE(blkhdr) == 0x01) {
		fprintf(stderr, "jalloc error (FATAL): double free detected at address %p!\n", ptr);
		abort();
	}

	JALLOC_SET_BLKHDR(blkhdr, order, 0x01);
	void * finalhdr = jcoalesce(blkhdr);
	jfreelist_pushblk(JALLOC_PAYLOAD(finalhdr));
}
