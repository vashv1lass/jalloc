#include "logic/segregated.h"

#include "cfg/conf.h"
#include "cfg/layout.h"
#include "os/mem.h"

static void * jfreelists_[JALLOC_ORDERCNT];

int
jmemrequest(void)
{
	void * chunkp = josmemrequest(JALLOC_CHUNKSZ);
	if (!chunkp) return -1; /* OOM */

	for (size_t offset = 0; offset < JALLOC_CHUNKSZ; offset += JALLOC_MAXBLKSZ) {
		void * blkhdr = (void *)((jalloc_byte_t *)chunkp + offset);
		JALLOC_SET_BLKHDR(blkhdr, JALLOC_MAXORDER, 0x01);
		jfreelist_pushblk(JALLOC_PAYLOAD(blkhdr));
	}

	return 0;
}

void
jfreelist_pushblk(void * blkp)
{
	uint8_t order = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp)) - JALLOC_MINORDER;
	void *	head  = jfreelists_[order];

	JALLOC_FREELISTPREV(blkp) = NULL;
	JALLOC_FREELISTNEXT(blkp) = head;

	if (head) JALLOC_FREELISTPREV(head) = blkp;

	jfreelists_[order] = blkp;
}

void
jfreelist_removeblk(void * blkp)
{
	uint8_t order	 = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp)) - JALLOC_MINORDER;
	void *	prevblkp = JALLOC_FREELISTPREV(blkp);
	void *	nextblkp = JALLOC_FREELISTNEXT(blkp);

	if (prevblkp) JALLOC_FREELISTNEXT(prevblkp) = nextblkp;
	if (nextblkp) JALLOC_FREELISTPREV(nextblkp) = prevblkp;

	if (!prevblkp) jfreelists_[order] = nextblkp;
}

void *
jfreelist_peek(uint8_t absorder)
{ return jfreelists_[absorder - JALLOC_MINORDER]; }
