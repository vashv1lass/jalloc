/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "os/testutils.h"

#include <stddef.h>

#if _POSIX_C_SOURCE >= 200809L

size_t
getpagesz(void)
{
    /* "SIZE_MAX on error" */
    return (size_t)sysconf(_SC_PAGESIZE);
}

#elif defined(_WIN32) /* && _POSIX_C_SOURCE < 200809L */

#include <windows.h>

size_t
getpagesz(void)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (size_t)sysinfo.dwPageSize;
}

#else /* !defined(_WIN32) && _POSIX_C_SOURCE < 200809L */

#error "Unsupported OS or unsupported OS version"

#endif /* _POSIX_C_SOURCE >= 200809L || defined(_WIN32) */