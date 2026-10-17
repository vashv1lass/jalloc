#ifndef JALLOC_CFG_CONF_H_
#define JALLOC_CFG_CONF_H_

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define JALLOC_ALIGN (_Alignof(max_align_t))

#define JALLOC_ALIGNUP(x, align) (((x) + (align) - 1) & ~((align) - 1))

#define JALLOC_MINORDER (5)
#define JALLOC_MAXORDER (20)

#define JALLOC_ORDERCNT (JALLOC_MAXORDER - JALLOC_MINORDER + 1)

#define JALLOC_MINBLKSZ (1 << JALLOC_MINORDER) /* 32 B */
#define JALLOC_MAXBLKSZ (1 << JALLOC_MAXORDER) /* 1 MiB */

#define JALLOC_CHUNKSZ (4 * JALLOC_MAXBLKSZ)

typedef uint8_t jalloc_byte_t; /* used for pointer arithmetic */

#endif /* JALLOC_CFG_CONF_H_ */
