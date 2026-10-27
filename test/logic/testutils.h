/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_TEST_LOGIC_TESTUTILS_H_
#define JALLOC_TEST_LOGIC_TESTUTILS_H_

#include <stddef.h>
#include <stdint.h>

/* helper for determining the appropriate free list size */
size_t
jfreelistsz(uint8_t order);

#endif /* !JALLOC_TEST_LOGIC_TESTUTILS_H_ */
