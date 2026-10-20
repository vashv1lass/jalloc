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

void
jfree(void * ptr)
{
	if (ptr == NULL) return;

	void *	blkhdr = JALLOC_BLKHDR(ptr);
	uint8_t order  = JALLOC_GET_ORDER(blkhdr);

	JALLOC_SET_BLKHDR(blkhdr, order, 0x01);
	void * finalhdr = jcoalesce(blkhdr);
	jfreelist_pushblk(JALLOC_PAYLOAD(finalhdr));
}
