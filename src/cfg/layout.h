/**
 * @file cfg/layout.h
 * @brief The config of the layout of the jalloc allocator.
 *
 * @author Maksim Vashkevich
 * @date 10/16/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JALLOC_CFG_LAYOUT_H_
#define JALLOC_CFG_LAYOUT_H_

#include "cfg/conf.h"

/**
 * @defgroup layout The allocator layout config
 * @internal
 *
 * @{
 */

/* 1 byte header and maxalign bytes padding (crying) */
#define JALLOC_BLKHDRSZ JALLOC_ALIGNUP(sizeof(uint8_t), JALLOC_ALIGN) /**< The size of the block header */

/**
 * @brief Gets the block header pointer from the block payload pointer.
 *
 * @param blkp The block payload pointer.
 *
 * @return The block header pointer.
 *
 * @note Function-like macro.
 */
#define JALLOC_BLKHDR(blkp) ((void *)((jalloc_byte_t *)(blkp) - JALLOC_BLKHDRSZ))

/**
 * @brief Gets the block payload pointer from the block header pointer.
 *
 * @param blkhdr The block header pointer.
 *
 * @return The block payload pointer.
 *
 * @note Function-like macro.
 */
#define JALLOC_PAYLOAD(blkhdr) ((void *)((jalloc_byte_t *)(blkhdr) + JALLOC_BLKHDRSZ))

/**
 * @defgroup block_header_struct The block header structure
 *
 * @brief The block header size is 1 byte and the other bytes are the alignment padding.
 * High nibble is the relative order (e.g. 2^JALLOC_MINORDER is the order 0, 2^(JALLOC_MINORDER + 1) is the order 1,
 * etc.)
 * 3 next bits are unused.
 * LSB of the header is the free bit (if the bit is set, then the block is free).
 *
 * @{
 */

/**
 * @brief Get the relative order from the header.
 *
 * @param blkhdr Pointer to a block header.
 *
 * @return The relative order.
 *
 * @note Function-like macro.
 */
#define JALLOC_GET_ORDER(blkhdr) (((*(uint8_t *)(blkhdr) & 0xF0) >> 4) + JALLOC_MINORDER)

/**
 * @brief Get the free bit from the header.
 *
 * @param blkhdr Pointer to a block header.
 *
 * @return The free bit of the block.
 *
 * @note Function-like macro.
 */
#define JALLOC_GET_FREE(blkhdr) (*(uint8_t *)(blkhdr) & 0x01)

/**
 * @brief Set the block header.
 *
 * @param blkhdr Pointer to a block header.
 * @param order Order of a block.
 * @param free The free bit value of a block.
 *
 * @return None.
 *
 * @note Function-like macro.
 */
#define JALLOC_SET_BLKHDR(blkhdr, order, free)                                                                         \
	(*(uint8_t *)(blkhdr) = (uint8_t)((order - JALLOC_MINORDER) << 4) | ((free) & 0x01))

/** @} */

/**
 * @defgroup free_block_struct The free block structure.
 *
 * @brief The free block payload consists of two pointers.
 * The first word is the pointer to the previous free block in the list.
 * The second word is the pointer to the next free block in the list.
 *
 * @{
 */

/**
 * @brief Get the previous free block in the free list.
 *
 * @param blkp Pointer to a block payload.
 *
 * @return A pointer to a previous free block in the list.
 *
 * @note Function-like macro.
 */
#define JALLOC_FREELISTPREV(blkp) (*(void **)(blkp))

/**
 * @brief Get the next free block in the free list.
 *
 * @param blkp Pointer to a block payload.
 *
 * @return A pointer to a next free block in the list.
 *
 * @note Function-like macro.
 */
#define JALLOC_FREELISTNEXT(blkp) (*(void **)((jalloc_byte_t *)(blkp) + sizeof(void *)))

/** @} */

/** @} */

#endif /* JALLOC_CFG_LAYOUT_H_ */
