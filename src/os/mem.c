#include "os/mem.h"

#include "cfg/conf.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

void *
josmemrequest(size_t sz)
{
	size_t memalign = JALLOC_MAXBLKSZ;
	size_t totalsz	= sz + memalign;

	uintptr_t raw_baseaddr, raw_endaddr;
	uintptr_t aligned_baseaddr, aligned_endaddr;

	ptrdiff_t prefsz, suffsz;

	void * raw;

	void * aligned;
	void * aligned_end;

	if ((raw = mmap(NULL, totalsz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		/* OOM */
		errno = ENOMEM;
		return NULL;
	}

	raw_baseaddr = (uintptr_t)raw;
	raw_endaddr  = raw_baseaddr + totalsz;

	aligned_baseaddr = JALLOC_ALIGNUP(raw_baseaddr, JALLOC_MAXBLKSZ);
	aligned_endaddr	 = aligned_baseaddr + sz;

	aligned	    = (void *)aligned_baseaddr;
	aligned_end = (void *)aligned_endaddr;

	prefsz = aligned_baseaddr - raw_baseaddr;
	suffsz = raw_endaddr - aligned_endaddr;

	if (prefsz > 0) munmap(raw, prefsz);
	if (suffsz > 0) munmap(aligned_end, suffsz);

	return aligned;
}
