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
 * @return Pointer to the beginning of the requested memory aligned by JALLOC_MAXBLKSZ and the page size on
 * success.
 * @return NULL on error (errno is set).
 * 
 * @note On POSIX systems (i.e. POSIX.1-2008) function will allocate strictly aligned \p sz bytes.
 * @note On Windows function will reserve (MEM_RESERVE) slightly more memory due to VirtualAlloc/VirtualFree system
 * calls specificity.
 */
void *
josmemrequest(size_t sz);

/**
 * @brief Returns memory to the OS.
 * 
 * @param ptr non-null pointer to memory to return, previously requested via josmemrequest.
 * @param sz bytes count to return (must be non-zero on POSIX, on Windows this parameter will be ignored).
 * 
 * @return 0 on success.
 * @return -1 on failure (errno is set).
 * 
 * @note On Windows, memory will be deallocated with MEM_RELEASE.
 */
int
josmemreturn(void * ptr, size_t sz); /* a reserve for the future */

/** @} */

#endif /* !JALLOC_OS_MEM_H_ */
