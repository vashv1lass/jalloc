/*
 * Copyright (c) 2026, Maksim Vashkevich. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logic/testbuddy.h"
#include "logic/testsegr.h"
#include "os/testmem.h"
#include "testjalloc.h"

#include <greatest/greatest.h>

GREATEST_MAIN_DEFS();

int
main(int argc, char * argv[])
{
	GREATEST_MAIN_BEGIN();

    /* NOTE: some tests assume the default allocator config */

	RUN_SUITE(josmem);
	RUN_SUITE(jsegr);
	RUN_SUITE(jbuddy);
	RUN_SUITE(jalloc_pub);

	GREATEST_MAIN_END();
}
