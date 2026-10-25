/**
 * @file os/mem.h
 * @brief OS/Allocator interaction interface.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_OS_MEM_H_
#define JALLOC_OS_MEM_H_

#include <stddef.h>

/**
 * @defgroup mem OS/Allocator interaction interface
 * @internal
 */

/** @{ */

/**
 * @brief Requests memory from the OS.
 *
 * @param sz bytes count to request (must be non-zero).
 *
 * @return Pointer to the beginning of the requested memory aligned by JALLOC_MAXBLKSZ and sysconf(_SC_PAGESIZE) on
 * success.
 * @return NULL on error (errno is set).
 */
void *
josmemrequest(size_t sz);

/** @} */

#endif /* !JALLOC_OS_MEM_H_ */
