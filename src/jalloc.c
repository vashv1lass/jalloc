#include "jalloc.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "logic/buddy.h"
#include "logic/segregated.h"
#include "os/mem.h"

#include <errno.h>
#include <stdint.h>

void *
jalloc(size_t sz)
{
	uint8_t targorder, factorder;
	void *	blkp = NULL;

	if (sz == 0) {
		errno = EINVAL;
		return NULL;
	}
	if ((targorder = jblkorder(sz + JALLOC_BLKHDRSZ)) > JALLOC_MAXORDER) {
		errno = ENOMEM;
		return NULL;
	}

	while (1) {
		for (factorder = targorder; factorder <= JALLOC_MAXORDER; factorder++) {
			blkp = jfreelist_peek(factorder);
			if (blkp) {
				jfreelist_removeblk(blkp);
				break;
			}
		}

		if (blkp) break;

		if (jmemrequest() == -1) return NULL; /* OOM */
	}

	jsplit(blkp, factorder, targorder);
	JALLOC_SET_BLKHDR(JALLOC_BLKHDR(blkp), targorder, 0x00);

	return blkp;
}
