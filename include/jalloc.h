/**
 * @file jalloc.h
 * @brief Public jalloc allocator interface.
 *
 * @author Maksim Vashkevich
 * @date 10/16/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_H_
#define JALLOC_H_

#include <stddef.h>

/**
 * @defgroup jalloc The public allocator interface.
 */

/** @{ */

/**
 * @brief Allocates certain amount of bytes on the heap.
 *
 * @param sz Bytes amount to allocate (non-zero).
 *
 * @return Pointer to the beginning of the allocated memory area on success.
 * @return NULL on error (errno is set).
 *
 * @note Function can allocate at most JALLOC_MAXBLKSZ - JALLOC_BLKHDRSZ (see src/cfg/conf.h) per one call.
 */
void *
jalloc(size_t sz);

/**
 * @brief Allocates certain count of elements of the certain size and initializes memory.
 *
 * @param nmemb Elements count to allocate (non-zero).
 * @param sz Size of each element (non-zero).
 *
 * @return Pointer to the first byte of the first allocated element on success.
 * @return NULL on error (errno is set).
 */
void *
jcalloc(size_t nmemb, size_t sz);

/**
 * @brief Reallocates (i.e. extends or reduces) the allocated memory area.
 *
 * @param ptr Pointer to the beginning of the area that was already allocated.
 * @param sz The size of the new memory area.
 *
 * @return Pointer to the beginning of the reallocated memory area on success.
 * @return NULL on error (errno is set) or on jrealloc(ptr, 0) call (see notes).
 *
 * @note jrealloc(NULL, sz) call is the equivalent of jalloc(sz) call (see jalloc docs).
 * @note jrealloc(ptr, 0) call is equivalent of jfree(ptr) call (see jfree docs).
 * @note Function is able to allocate at most JALLOC_MAXBLKSZ - JALLOC_BLKHDRSZ per call.
 */
void *
jrealloc(void * ptr, size_t sz);

/**
 * @brief Deallocates the allocated area (but does not null it).
 *
 * @param ptr Pointer to the beginning of the memory area to deallocate (must be allocated via jalloc).
 *
 * @note If the pointer is NULL, then function safely does not do anything.
 * @note If the pointer was not allocated via jalloc, then the behaviour is undefined.
 * @note If the pointer was already freed via jfree, then the process aborts.
 */
void
jfree(void * ptr);

/** @} */

#endif /* JALLOC_H_ */
