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
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

void *
josmemrequest(size_t sz)
{
	size_t align = JALLOC_MAXBLKSZ;
	if (sz > SIZE_MAX - align) {
		errno = EOVERFLOW;
		return NULL;
	}
	size_t totalsz = sz + align;

	void * raw = mmap(NULL, totalsz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (raw == MAP_FAILED) return NULL;

	uintptr_t raw_base_addr = (uintptr_t)raw;
	uintptr_t raw_end_addr	= raw_base_addr + totalsz;

	uintptr_t aligned_base_addr = JALLOC_ALIGNUP(raw_base_addr, align);
	uintptr_t aligned_end_addr  = aligned_base_addr + sz;

	size_t prefsz = (size_t)(aligned_base_addr - raw_base_addr);
	size_t suffsz = (size_t)(raw_end_addr - aligned_end_addr);

	if (prefsz > 0 && munmap((void *)raw_base_addr, prefsz) == -1) {
		/* unmap the memory if something goes wrong */
		munmap((void *)raw_base_addr, totalsz);
		return NULL;
	}
	if (suffsz > 0 && munmap((void *)aligned_end_addr, suffsz) == -1) {
		munmap((void *)aligned_base_addr, totalsz - prefsz); /* the prefix is already unmapped */
		return NULL;
	}

	return (void *)aligned_base_addr;
}
