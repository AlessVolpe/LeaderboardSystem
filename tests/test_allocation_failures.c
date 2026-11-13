/*
 * LeaderboardSystem deterministic allocation-failure regressions (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ranking.h"
#include "hashmap_base.h"
#include "alloc_internal.h"

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #test); \
    return EXIT_FAILURE; } } while (0)

static size_t hash_key(const void *key) { return hashmap_hash_string(key); }
static int compare_key(const void *a, const void *b) { return strcmp(a, b); }

int main(void) {
    for (size_t i = 0; i < 4; ++i) {
        leaderboard_alloc_fail_after(i);
        ranking_t *ranking = ranking_new();
        leaderboard_alloc_reset();
        if (ranking != NULL) ranking_destroy(ranking);
    }
    for (size_t i = 0; i < 3; ++i) {
        leaderboard_alloc_fail_after(i);
        hashmap_base_t *map = hashmap_base_new(hash_key, compare_key);
        leaderboard_alloc_reset();
        hashmap_base_destroy(map);
    }
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    CHECK(ranking_upsert(ranking, "alice", 10) == 0);
    CHECK(ranking_upsert(ranking, "bob", 7) == 0);
    for (size_t i = 0; i < 6; ++i) {
        leaderboard_alloc_fail_after(i);
        int rc = ranking_upsert(ranking, "charlie", 12);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        int64_t score = 0;
        size_t rank = 0;
        CHECK(ranking_get(ranking, "alice", &score, &rank) == 0);
        CHECK(score == 10);
        CHECK(ranking_get(ranking, "bob", &score, &rank) == 0);
        CHECK(score == 7);
        CHECK(ranking_size(ranking) == (rc == 0 ? 3u : 2u));
        if (rc == 0) CHECK(ranking_remove(ranking, "charlie") == 0);
    }
    for (size_t i = 0; i < 5; ++i) {
        leaderboard_alloc_fail_after(i);
        int rc = ranking_upsert(ranking, "bob", 100);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        int64_t score = 0;
        size_t rank = SIZE_MAX;
        CHECK(ranking_get(ranking, "bob", &score, &rank) == 0);
        CHECK(score == (rc == 0 ? 100 : 7));
        CHECK(rank == (rc == 0 ? 0u : 1u));
        if (rc == 0) CHECK(ranking_upsert(ranking, "bob", 7) == 0);
        CHECK(ranking_size(ranking) == 2);
        const char *id = NULL;
        CHECK(ranking_at(ranking, 0, &id, NULL) == 0);
        CHECK(strcmp(id, "alice") == 0);
    }
    ranking_destroy(ranking);

    hashmap_base_t *map = hashmap_base_new(hash_key, compare_key);
    CHECK(map != NULL);
    for (size_t i = 0; i < 3; ++i) {
        leaderboard_alloc_fail_after(i);
        int rc = hashmap_base_put(map, "key", "value");
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        if (rc == 0) CHECK(hashmap_base_remove(map, "key") != NULL);
        CHECK(hashmap_base_size(map) == 0);
    }
    hashmap_base_destroy(map);
    return EXIT_SUCCESS;
}
