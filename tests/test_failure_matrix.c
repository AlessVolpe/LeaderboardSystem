/*
 * LeaderboardSystem failure-atomicity and lifecycle matrix (milestone 3.0.8).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc_internal.h"
#include "bostree.h"
#include "hashmap_base.h"
#include "ranking.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    abort(); } } while (0)

#define RANK_COUNT 32u
#define MAP_COUNT 96u
#define SWEEP_LIMIT 24u

static size_t colliding_hash(const void *key) {
    (void)key;
    return 17u;
}
static int compare_key(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static void verify_ranking(ranking_t *ranking, unsigned int count, int with_new,
                           int with_update) {
    size_t expected_size = count + (with_new != 0 ? 1u : 0u);
    CHECK(ranking_size(ranking) == expected_size);
    for (unsigned int i = 0; i < count; ++i) {
        char id[24];
        (void)snprintf(id, sizeof(id), "p%03u", i);
        int64_t score = INT64_MIN;
        size_t rank = SIZE_MAX;
        CHECK(ranking_get(ranking, id, &score, &rank) == 0);
        CHECK(score == ((with_update && i == 0u) ? 9999 : (int64_t)i));
        size_t expected_rank = count - i - 1u;
        if (with_update && i == 0u) expected_rank = 0;
        else if (with_update) expected_rank++;
        if (with_new) expected_rank++; /* New entry is always above all original scores. */
        CHECK(rank == expected_rank);
    }
    for (size_t r = 0; r < expected_size; ++r) {
        const char *id = NULL;
        int64_t score = INT64_MIN;
        CHECK(ranking_at(ranking, r, &id, &score) == 0);
        CHECK(id != NULL);
        int64_t retrieved = INT64_MIN;
        size_t reverse_rank = SIZE_MAX;
        CHECK(ranking_get(ranking, id, &retrieved, &reverse_rank) == 0);
        CHECK(retrieved == score && reverse_rank == r);
    }
}

static ranking_t *populated_ranking(unsigned int count) {
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    for (unsigned int i = 0; i < count; ++i) {
        char id[24];
        (void)snprintf(id, sizeof(id), "p%03u", i);
        CHECK(ranking_upsert(ranking, id, (int64_t)i) == 0);
    }
    return ranking;
}

static void ranking_fault_sweeps(void) {
    unsigned int failures = 0, successes = 0;
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        leaderboard_alloc_fail_after(fault);
        ranking_t *ranking = ranking_new();
        leaderboard_alloc_reset();
        if (ranking == NULL) ++failures;
        else { ++successes; CHECK(ranking_size(ranking) == 0); ranking_destroy(ranking); }
    }
    CHECK(failures != 0 && successes != 0);

    failures = 0;
    successes = 0;
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        ranking_t *ranking = populated_ranking(RANK_COUNT);
        verify_ranking(ranking, RANK_COUNT, 0, 0);
        leaderboard_alloc_fail_after(fault);
        int rc = ranking_upsert(ranking, "new-top", 100000);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        if (rc == 0) ++successes; else ++failures;
        verify_ranking(ranking, RANK_COUNT, rc == 0, 0);
        if (rc == 0) CHECK(ranking_remove(ranking, "new-top") == 0);
        verify_ranking(ranking, RANK_COUNT, 0, 0);
        ranking_destroy(ranking);
    }
    CHECK(failures != 0 && successes != 0);

    failures = 0;
    successes = 0;
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        ranking_t *ranking = populated_ranking(RANK_COUNT);
        leaderboard_alloc_fail_after(fault);
        int rc = ranking_upsert(ranking, "p000", 9999);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        if (rc == 0) ++successes; else ++failures;
        verify_ranking(ranking, RANK_COUNT, 0, rc == 0);
        if (rc == 0) CHECK(ranking_upsert(ranking, "p000", 0) == 0);
        verify_ranking(ranking, RANK_COUNT, 0, 0);
        /* Identical-score upsert is allocation-free and succeeds under a fault. */
        leaderboard_alloc_fail_after(0);
        CHECK(ranking_upsert(ranking, "p000", 0) == 0);
        leaderboard_alloc_reset();
        ranking_destroy(ranking);
    }
    CHECK(failures != 0 && successes != 0);
}

static char keys[MAP_COUNT + 1][16];
static int values[MAP_COUNT + 1];

static void verify_map(hashmap_base_t *map, unsigned int count, int has_new) {
    CHECK(hashmap_base_size(map) == count + (has_new != 0 ? 1u : 0u));
    for (unsigned int i = 0; i < count; ++i)
        CHECK(hashmap_base_get(map, keys[i]) == &values[i]);
    CHECK(hashmap_base_get(map, keys[count]) == (has_new ? &values[count] : NULL));
}

static void hashmap_rehash_fault_sweep(void) {
    for (unsigned int i = 0; i <= MAP_COUNT; ++i) {
        (void)snprintf(keys[i], sizeof(keys[i]), "key-%03u", i);
        values[i] = (int)i;
    }
    unsigned int failures = 0, successes = 0;
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        hashmap_base_t *map = hashmap_base_new(colliding_hash, compare_key);
        CHECK(map != NULL);
        for (unsigned int i = 0; i < MAP_COUNT; ++i)
            CHECK(hashmap_base_put(map, keys[i], &values[i]) == 0);
        verify_map(map, MAP_COUNT, 0);
        leaderboard_alloc_fail_after(fault);
        int rc = hashmap_base_put(map, keys[MAP_COUNT], &values[MAP_COUNT]);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        if (rc == 0) ++successes; else ++failures;
        verify_map(map, MAP_COUNT, rc == 0);
        if (rc == 0) CHECK(hashmap_base_remove(map, keys[MAP_COUNT]) == &values[MAP_COUNT]);
        verify_map(map, MAP_COUNT, 0);
        hashmap_base_destroy(map);
    }
    CHECK(failures != 0 && successes != 0);
}

/* A callback-controlled failure also checks that successfully duplicated
 * keys are released once, including after failed rehash attempts. */
typedef struct owner_stats {
    unsigned int copies;
    unsigned int releases;
} owner_stats_t;

static size_t context_hash(const void *key, void *context) {
    (void)key; (void)context;
    return 1u;
}
static int context_compare(const void *a, const void *b, void *context) {
    (void)context;
    return strcmp((const char *)a, (const char *)b);
}
static void *context_dup(const void *key, void *context) {
    owner_stats_t *stats = context;
    size_t len = strlen((const char *)key) + 1u;
    char *copy = leaderboard_alloc_malloc(len);
    if (copy != NULL) { memcpy(copy, key, len); ++stats->copies; }
    return copy;
}
static void context_release(void *key, void *context) {
    owner_stats_t *stats = context;
    ++stats->releases;
    free(key);
}

static void hashmap_ownership_fault_sweep(void) {
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        owner_stats_t stats = {0};
        hashmap_base_t *map = hashmap_base_new_with_context(
            context_hash, context_compare, &stats, NULL);
        CHECK(map != NULL);
        CHECK(hashmap_base_set_context_key_alloc_funcs(map, context_dup, context_release) == 0);
        CHECK(hashmap_base_put(map, "stable", &values[0]) == 0);
        leaderboard_alloc_fail_after(fault);
        int rc = hashmap_base_put(map, "candidate", &values[1]);
        leaderboard_alloc_reset();
        CHECK(rc == 0 || rc == -ENOMEM);
        CHECK(hashmap_base_get(map, "stable") == &values[0]);
        CHECK(hashmap_base_get(map, "candidate") == (rc == 0 ? &values[1] : NULL));
        hashmap_base_destroy(map);
        CHECK(stats.copies == stats.releases);
    }
}

static unsigned int finalized[4];
static int tree_keys[4] = {10, 20, 30, 40};

static int compare_int(const void *a, const void *b) {
    int lhs = *(const int *)a, rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}
static void finalize_node(bostree_node_t *node) {
    CHECK(!bostree_node_is_valid(node));
    const int *key = bostree_node_key(node);
    CHECK(key != NULL);
    CHECK(bostree_node_data(node) != NULL);
    size_t index = (size_t)((const int *)bostree_node_data(node) - tree_keys);
    CHECK(index < 4u && key == &tree_keys[index]);
    CHECK(finalized[index]++ == 0u);
}

static void tree_lifecycle_fault_sweep(void) {
    for (size_t fault = 0; fault < SWEEP_LIMIT; ++fault) {
        memset(finalized, 0, sizeof(finalized));
        leaderboard_alloc_fail_after(fault);
        bostree_t *tree = bostree_new(compare_int, finalize_node);
        leaderboard_alloc_reset();
        if (tree == NULL) continue;
        bostree_node_t *a = bostree_insert(tree, &tree_keys[0], &tree_keys[0]);
        bostree_node_t *b = bostree_insert(tree, &tree_keys[1], &tree_keys[1]);
        CHECK(a != NULL && b != NULL);
        bostree_node_t *held = bostree_node_weak_ref(a);
        CHECK(held == a);
        leaderboard_alloc_fail_after(fault);
        bostree_node_t *c = bostree_insert(tree, &tree_keys[2], &tree_keys[2]);
        leaderboard_alloc_reset();
        CHECK(bostree_node_count(tree) == (c == NULL ? 2u : 3u));
        CHECK(bostree_select(tree, 0) == a);
        CHECK(bostree_lookup(tree, &tree_keys[0]) == a);
        CHECK(bostree_node_is_valid(held));
        bostree_destroy(tree);
        CHECK(!bostree_node_is_valid(held));
        CHECK(bostree_node_key(held) == NULL);
        CHECK(bostree_node_data(held) == NULL);
        CHECK(finalized[0] == 1u && finalized[1] == 1u);
        CHECK(finalized[2] == (c == NULL ? 0u : 1u));
        CHECK(bostree_node_weak_unref(NULL, held) == NULL);
    }
}

int main(void) {
    ranking_fault_sweeps();
    hashmap_rehash_fault_sweep();
    hashmap_ownership_fault_sweep();
    tree_lifecycle_fault_sweep();
    return EXIT_SUCCESS;
}
