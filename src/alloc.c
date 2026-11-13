/*
 * LeaderboardSystem allocation boundary (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "alloc_internal.h"
#include <stdlib.h>

#ifdef LEADERBOARD_TEST_ALLOCATION_FAULTS
#include <stdint.h>
static size_t remaining = SIZE_MAX;

void leaderboard_alloc_fail_after(size_t successes) {
    remaining = successes;
}

void leaderboard_alloc_reset(void) {
    remaining = SIZE_MAX;
}

static int should_fail(void) {
    if (remaining == SIZE_MAX) return 0;
    if (remaining-- != 0) return 0;
    remaining = SIZE_MAX;  /* One-shot failure. */
    return 1;
}
#else
static int should_fail(void) { return 0; }
#endif

void *leaderboard_alloc_malloc(size_t size) {
    return should_fail() ? NULL : malloc(size);
}

void *leaderboard_alloc_calloc(size_t count, size_t size) {
    return should_fail() ? NULL : calloc(count, size);
}
