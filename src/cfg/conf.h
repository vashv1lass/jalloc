/**
 * @file cfg/conf.h
 * @brief The main config of the jalloc allocator.
 *
 * @author Maksim Vashkevich
 * @date 10/16/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_CFG_CONF_H_
#define JALLOC_CFG_CONF_H_

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup conf The allocator config
 * @internal
 */

/** @{ */

#if defined(_MSC_VER) && !defined(__clang__)

/* love msvc very much xd */
typedef union {
    long long   ll;
    long double ld;
    void *       p;
    void         (*fnp)(void);
} jalloc_max_align_t;

#define JALLOC_ALIGN (_Alignof(jalloc_max_align_t)) /**< The system-specific alignment. */

#else /* !_MSC_VER || __clang__ */
#define JALLOC_ALIGN (_Alignof(max_align_t)) /**< The system-specific alignment. */
#endif /* _MSC_VER && !__clang__ */

/**
 * @brief Aligns the value up to certain alignment parameter.
 *
 * @param x The value to align up.
 * @param align The alignment parameter.
 *
 * @return The aligned to \p align value of \p x.
 *
 * @note Function-like macro.
 */
#define JALLOC_ALIGNUP(x, align) (((x) + (align) - 1) & ~((align) - 1))

#define JALLOC_MINORDER (5)  /**< Minimal absolute order of a block */
#define JALLOC_MAXORDER (20) /**< Maximal absolute order of a block */

#define JALLOC_ORDERCNT (JALLOC_MAXORDER - JALLOC_MINORDER + 1) /**< Block order count */

/* Only 4 bits are used to store the order of the block */
_Static_assert(1 <= JALLOC_ORDERCNT && JALLOC_ORDERCNT <= 16, "Order count must be between 1 and 16");

#define JALLOC_MINBLKSZ (1 << JALLOC_MINORDER) /**< Minimal block size */
#define JALLOC_MAXBLKSZ (1 << JALLOC_MAXORDER) /**< Maximal block size */

#define JALLOC_CHUNKSZ (4 * JALLOC_MAXBLKSZ) /**< Allocation chunk size */

/* This assertion is requirement of the block alignment */
_Static_assert(
    JALLOC_CHUNKSZ % JALLOC_MAXBLKSZ == 0, "Allocator chunk size must be a multiple of allocator max block size");

typedef uint8_t jalloc_byte_t; /**< The type used for a pointer arithmetic */

/** @} */

#endif /* !JALLOC_CFG_CONF_H_ */
