#include "logic/buddy.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "segregated.h"

#include <stddef.h>
#include <stdint.h>

void
jsplit(void * blkp, uint8_t factorder, uint8_t targorder)
{
	while (factorder-- > targorder) {
		size_t halfsz	   = 1 << factorder;
		void * rhsbuddy = (void *)((jalloc_byte_t *)blkp + halfsz);

		JALLOC_SET_BLKHDR(JALLOC_BLKHDR(rhsbuddy), factorder, 0x01);
		jfreelist_pushblk(rhsbuddy);
	}
}
