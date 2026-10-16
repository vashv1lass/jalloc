#ifndef JALLOC_CFG_LAYOUT_H_
#define JALLOC_CFG_LAYOUT_H_

#include "cfg/conf.h"

/* 1 byte header and maxalign bytes padding (crying) */
#define JALLOC_BLKHDRSZ JALLOC_ALIGNUP(sizeof(uint8_t))

#define JALLOC_BLKHDR(blkp) ((void *)((jalloc_byte_t *)(blkp) - JALLOC_BLKHDRSZ))

/*
 * block header structure:
 * high nibble - order (2^5 - order 0, 2^6 - order 1, etc.)
 * 3 bits - unused
 * lsb - free bit
 */
#define JALLOC_GET_ORDER(blkhdr) ((*(uint8_t *)(blkhdr) & 0xF0) >> 4)
#define JALLOC_GET_FREE(blkhdr)	 (*(uint8_t *)(blkhdr) & 0x01)

#define JALLOC_SET_BLKHDR(blkhdr, order, free) (*(uint8_t *)(blkhdr) = ((order) << 4) | ((free) & 0x01))

/*
 * free block structure (use this macros only on free blocks!):
 * prev free list element pointer (8 bytes)
 * next free list element pointer (8 bytes)
 */
#define JALLOC_FREE_LISTPREV(blkp) (*(void **)(blkp))
#define JALLOC_FREE_LISTNEXT(blkp) (*(void **)((jalloc_byte_t *)(blkp) + sizeof(void *)))

#endif /* JALLOC_CFG_LAYOUT_H_ */
