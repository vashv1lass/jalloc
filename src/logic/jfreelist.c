#include "jfreelist.h"

#include "cfg/conf.h"
#include "cfg/layout.h"

static void * jfreelists[JALLOC_ORDERCNT];

void
jfreelist_pushblk(void * blkp)
{
	uint8_t order = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp));
	void *	head  = jfreelists[order];

	JALLOC_FREELISTPREV(blkp) = NULL;
	JALLOC_FREELISTNEXT(blkp) = head;

	if (head) JALLOC_FREELISTPREV(head) = blkp;

	jfreelists[order] = blkp;
}

void
jfreelist_removeblk(void * blkp)
{
	uint8_t order	 = JALLOC_GET_ORDER(JALLOC_BLKHDR(blkp));
	void *	prevblkp = JALLOC_FREELISTPREV(blkp);
	void *	nextblkp = JALLOC_FREELISTNEXT(blkp);

	if (prevblkp) JALLOC_FREELISTNEXT(prevblkp) = nextblkp;
	if (nextblkp) JALLOC_FREELISTPREV(nextblkp) = prevblkp;

	if (!prevblkp) jfreelists[order] = nextblkp;
}

void *
jfreelist_peek(uint8_t order)
{ return jfreelists[order]; }
