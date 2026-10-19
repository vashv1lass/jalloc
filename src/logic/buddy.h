/**
 * @file logic/buddy.h
 * @brief Buddy system related algorithms.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_LOGIC_BUDDY_H_
#define JALLOC_LOGIC_BUDDY_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup buddy Buddy system related algorithms.
 * @internal
 *
 * @{
 */

/**
 * @brief Successively splits the block in half until the desired size is reached.
 *
 * @param blkp Pointer to the block to split.
 * @param factorder Absolute order of a \p blkp unsplit block (<= JALLOC_MAXORDER).
 * @param targorder Order of a \p blkp block to reach (<= JALLOC_MAXORDER, >= \p targorder).
 *
 * @return None.
 *
 * @note When the block is split, the rhs block (a.k.a. "buddy") is marked as free and the lhs block is split again.
 */
void
jsplit(void * blkp, uint8_t factorder, uint8_t targorder);

/** @} */

#endif /* JALLOC_LOGIC_BUDDY_H_ */
