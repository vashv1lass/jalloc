/**
 * @file os/mem.c
 * @brief OS/Allocator interaction interface functions definitions.
 *
 * @author Maksim Vashkevich
 * @date 10/17/26
 *
 * @copyright Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/mem.h"

#include "cfg/conf.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

void *
josmemrequest(size_t sz)
{
	/*
	 * defensive programming paths after system calls like `mmap()` of `open()` won't be covered by unit tests
	 * (no reason for doing such things)
	 */
	if (sz == 0) {
		errno = EINVAL;
		return NULL;
	}

	size_t const PAGESZ = sysconf(_SC_PAGESIZE);
	size_t	     align  = JALLOC_MAXBLKSZ > PAGESZ ? JALLOC_MAXBLKSZ : PAGESZ;
	if (sz > SIZE_MAX - align) {
		errno = EOVERFLOW;
		return NULL;
	}
	size_t totalsz = sz + align;

	int anonymous_fd = open("/dev/zero", O_RDWR);
	if (anonymous_fd == -1) return NULL;

	void * raw = mmap(NULL, totalsz, PROT_READ | PROT_WRITE, MAP_PRIVATE, anonymous_fd, 0);
	close(anonymous_fd); /* won't set errno (/dev/zero close is safe) */

	if (raw == MAP_FAILED) return NULL;

	uintptr_t raw_base_addr = (uintptr_t)raw;
	uintptr_t raw_end_addr	= raw_base_addr + totalsz;

	uintptr_t aligned_base_addr = JALLOC_ALIGNUP(raw_base_addr, align);
	uintptr_t aligned_end_addr  = aligned_base_addr + sz;

	size_t prefsz = (size_t)(aligned_base_addr - raw_base_addr);
	size_t suffsz = (size_t)(raw_end_addr - aligned_end_addr);

	/*
	 * this logic won't be covered w/ unit-tests because there's no reason to mock munmap logic (it will bloat the
	 * testing code).
	 *
	 * this code is mathematically perfect, so its fine not to test it.
	 */
	if (prefsz > 0 && munmap((void *)raw_base_addr, prefsz) == -1) {
		/* unmap the memory if something goes wrong */
		munmap((void *)raw_base_addr, totalsz); /* won't set errno */
		return NULL;
	}
	if (suffsz > 0 && munmap((void *)aligned_end_addr, suffsz) == -1) {
		/* the prefix is already unmapped */
		munmap((void *)aligned_base_addr, totalsz - prefsz); /* won't set errno */
		return NULL;
	}

	return (void *)aligned_base_addr;
}
