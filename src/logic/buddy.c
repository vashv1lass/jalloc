/**
 * @file logic/buddy.c
 * @brief Buddy system related algorithms functions definitions.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/buddy.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "segregated.h"

#include <stddef.h>
#include <stdint.h>

void
jsplit(void * blkp, uint8_t factorder, uint8_t targorder)
{
	while (factorder-- > targorder) {
		void * rhsbuddy = (void *)((jalloc_byte_t *)blkp + ((ptrdiff_t)1 << factorder));

		JALLOC_SET_BLKHDR(JALLOC_BLKHDR(rhsbuddy), factorder, 0x01);
		jfreelist_pushblk(rhsbuddy);
	}
}

void *
jcoalesce(void * blkhdr)
{
	for (uint8_t order = JALLOC_GET_ORDER(blkhdr); order < JALLOC_MAXORDER; order++) {
		void * buddyhdr = (void *)((uintptr_t)blkhdr ^ (1 << order));
		if (!JALLOC_GET_FREE(buddyhdr) || JALLOC_GET_ORDER(buddyhdr) != order) break;

		jfreelist_removeblk(JALLOC_PAYLOAD(buddyhdr));
		if ((uintptr_t)buddyhdr < (uintptr_t)blkhdr) blkhdr = buddyhdr;
		JALLOC_SET_BLKHDR(blkhdr, order + 1, 0x01);
	}

	return blkhdr;
}
