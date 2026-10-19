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
 * @brief Allocates certain amount of bytes on the heap.
 *
 * @param sz Bytes amount to allocate.
 *
 * @return Pointer to the beginning of the allocated memory area.
 */
void *
jalloc(size_t sz);

#endif /* JALLOC_H_ */
