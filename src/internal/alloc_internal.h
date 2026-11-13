/*
 * LeaderboardSystem allocation boundary (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 * Internal implementation and deterministic failure testing only.
 */
#ifndef LEADERBOARD_ALLOC_INTERNAL_H
#define LEADERBOARD_ALLOC_INTERNAL_H

#include <stddef.h>

void *leaderboard_alloc_malloc(size_t size);
void *leaderboard_alloc_calloc(size_t count, size_t size);

#ifdef LEADERBOARD_TEST_ALLOCATION_FAULTS
/* The next allocation after `successes` successful attempts fails once.
 * Global, deterministic, single-threaded testing hook; never installed. */
void leaderboard_alloc_fail_after(size_t successes);
void leaderboard_alloc_reset(void);
#endif

#endif /* LEADERBOARD_ALLOC_INTERNAL_H */
