/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_TEST_OS_TESTUTILS_H_
#define JALLOC_TEST_OS_TESTUTILS_H_

#include <stddef.h>

/* cross-platform helper to determine the system page size */
/* will return page size or SIZE_MAX on error */
size_t
getpagesz(void);

#endif /* JALLOC_TEST_OS_TESTUTILS_H_ */
