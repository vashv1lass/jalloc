/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/testutils.h"

#include "cfg/layout.h"
#include "logic/segregated.h"

size_t
jfreelistsz(uint8_t order)
{
	size_t sz     = 0;
	void * curblk = jfreelist_peek(order);
	for (; curblk != NULL; sz++, curblk = JALLOC_FREELISTNEXT(curblk));
	return sz;
}
