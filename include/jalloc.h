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

#if defined(_MSC_VER)

#if defined(JALLOC_BUILD_SHARED)
#define JALLOC_PUBAPI __declspec(dllexport)
#elif defined(JALLOC_DLL) /* && !JALLOC_BUILD_SHARED */
#define JALLOC_PUBAPI __declspec(dllimport)
#else /* !JALLOC_BUILD_SHARED && !JALLOC_DLL */
#define JALLOC_PUBAPI
#endif /* !JALLOC_BUILD_SHARED */

#elif defined(__has_attribute) && __has_attribute(visibility) /* && !_MSC_VER */

#define JALLOC_PUBAPI __attribute__((visibility("default")))

#else /* !visibility && !_MSC_VER */

#define JALLOC_PUBAPI

#endif /* _MSC_VER */

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
 * @note Function can allocate at most 1 MiB per one call in default configuration (see src/cfg/conf.h).
 */
JALLOC_PUBAPI void *
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
JALLOC_PUBAPI void *
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
 * @note Function is able to allocate at most 1 MiB per call in default configuration.
 */
JALLOC_PUBAPI void *
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
JALLOC_PUBAPI void
jfree(void * ptr);

/** @} */

#endif /* JALLOC_H_ */
