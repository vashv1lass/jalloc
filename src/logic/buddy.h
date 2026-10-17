#ifndef JALLOC_LOGIC_BUDDY_H_
#define JALLOC_LOGIC_BUDDY_H_

#include <stddef.h>
#include <stdint.h>

void
jsplit(void * blkp, uint8_t factorder, uint8_t targorder);

#endif /* JALLOC_LOGIC_BUDDY_H_ */
