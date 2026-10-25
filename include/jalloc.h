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
 * @param sz Bytes amount to allocate.
 *
 * @return Pointer to the beginning of the allocated memory area on success.
 * @return NULL on error (errno is set)
 *
 * @note Function can allocate at most JALLOC_MAXBLKSZ - JALLOC_BLKHDRSZ per one call.
 */
void *
jalloc(size_t sz);

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
