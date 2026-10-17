#ifndef JALLOC_FREELIST_H_
#define JALLOC_FREELIST_H_

#include <stdint.h>

void
jfreelist_pushblk(void * blkp);

void
jfreelist_removeblk(void * blkp);

void *
jfreelist_peek(uint8_t order);

#endif /* JALLOC_FREELIST_H_ */
