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

#if _POSIX_C_SOURCE >= 200809L

#include <fcntl.h>
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

	long pagesz_conf = sysconf(_SC_PAGESIZE);
	if (pagesz_conf == -1) return NULL;
	size_t const PAGESZ = (size_t)pagesz_conf;

	if (sz % PAGESZ != 0) {
		errno = EINVAL;
		return NULL;
	}

	size_t align = JALLOC_MAXBLKSZ > PAGESZ ? JALLOC_MAXBLKSZ : PAGESZ;
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

	if (raw_base_addr > UINTPTR_MAX - align + 1) {
		errno = EOVERFLOW;
		return NULL;
	}

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

int
josmemreturn(void * ptr, size_t sz)
{
	if (ptr == NULL || sz == 0) {
		errno = EINVAL;
		return -1;
	}

	return munmap(ptr, sz);
}

#elif defined(_WIN32) /* && _POSIX_C_SOURCE < 200809L */

#include "cfg/compiler.h"

#include <windows.h>

void *
josmemrequest(size_t sz)
{
	if (sz == 0) {
		errno = EINVAL;
		return NULL;
	}

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	size_t const PAGESZ = (size_t)sysinfo.dwPageSize;

	if (sz % PAGESZ != 0) {
		errno = EINVAL;
		return NULL;
	}

	size_t align = JALLOC_MAXBLKSZ > PAGESZ ? JALLOC_MAXBLKSZ : PAGESZ;
	if (sz > SIZE_MAX - align) {
		errno = EOVERFLOW;
		return NULL;
	}
	size_t totalsz = sz + align;

	/* VirtualQuery() + mbi.AllocationBase will restore this pointer */
	void * raw = VirtualAlloc(NULL, totalsz, MEM_RESERVE, PAGE_NOACCESS);
	if (raw == NULL) {
		/* no reason to GetLastError() there */
		errno = ENOMEM;
		return NULL;
	}

	uintptr_t raw_base_addr = (uintptr_t)raw;

	if (raw_base_addr > UINTPTR_MAX - align + 1) {
		errno = EOVERFLOW;
		return NULL;
	}

	uintptr_t aligned_base_addr = JALLOC_ALIGNUP(raw_base_addr, align);

	if (VirtualAlloc((void *)aligned_base_addr, sz, MEM_COMMIT, PAGE_READWRITE) == NULL) {
		VirtualFree(raw, 0, MEM_RELEASE);
		errno = ENOMEM;
		return NULL;
	}

	/* unfortunately, alignment artifact will remain reserved */
	return (void *)aligned_base_addr;
}

int
josmemreturn(void * ptr, size_t sz)
{
	JALLOC_ARGIGNORE(sz);

	if (ptr == NULL) {
		errno = EINVAL;
		return -1;
	}

	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) {
		/* this code is likely dead, so it's defensive programming */
		errno = EINVAL;
		return -1;
	}

	if (!VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE)) {
		errno = EFAULT;
		return -1;
	}

	return 0;
}

#else /* !defined(_WIN32) && _POSIX_C_SOURCE < 200809L */

#error "Unsupported OS or unsupported OS version"

#endif /* _POSIX_C_SOURCE >= 200809L || defined(_WIN32) */
