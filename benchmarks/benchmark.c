/*
 * LeaderboardSystem deterministic public-API benchmarks (milestone 3.0.10).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Times portable C11 clock() processor time, not wall time. Operation checks
 * are deliberately included in the measurement. No throughput pass/fail
 * threshold is used; see docs/performance-release-3.0.10.md.
 */
#include "bostree.h"
#include "hashmap_base.h"
#include "ranking.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ENTRIES 100000U
#define MAX_ROUNDS 30U
#define ID_LENGTH 24U

typedef struct workload {
    size_t count;
    char (*ids)[ID_LENGTH];
    uint32_t *permutation;
    uint32_t *keys;
    uint64_t *values;
    int64_t *scores;
    bostree_node_t **nodes;
} workload_t;

static volatile uint64_t observed_checksum;

static void require(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "benchmark error: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static size_t parse_positive(const char *arg, size_t maximum) {
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' || arg[0] == '-' ||
        value == 0UL || value > maximum) {
        fprintf(stderr, "invalid positive integer '%s' (maximum %zu)\n", arg, maximum);
        exit(EXIT_FAILURE);
    }
    return (size_t)value;
}

/* A fixed, non-cryptographic PRNG for reproducible operation order. */
static uint32_t next_random(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void prepare(workload_t *w, size_t count, size_t round) {
    uint32_t rng = UINT32_C(0xC2B2AE35) ^ (uint32_t)(round + 1U);
    w->count = count;
    for (size_t i = 0; i < count; ++i) {
        /* Each identifier is unique and retains its address for the trial. */
        int written = snprintf(w->ids[i], ID_LENGTH, "player-%08" PRIx32, (uint32_t)i);
        require(written > 0 && (size_t)written < ID_LENGTH, "ID formatting failed");
        w->permutation[i] = (uint32_t)i;
        w->keys[i] = (uint32_t)i;
        w->values[i] = (uint64_t)i + UINT64_C(1);
        w->scores[i] = (int64_t)(i % 2001U) - INT64_C(1000);
        w->nodes[i] = NULL;
    }
    for (size_t i = count - 1U; i > 0U; --i) {
        size_t j = (size_t)(next_random(&rng) % (uint32_t)(i + 1U));
        uint32_t tmp = w->permutation[i];
        w->permutation[i] = w->permutation[j];
        w->permutation[j] = tmp;
    }
}

static double elapsed(clock_t start) {
    clock_t end = clock();
    require(start != (clock_t)-1 && end != (clock_t)-1 && end >= start,
            "clock() unavailable or wrapped");
    return (double)(end - start) / (double)CLOCKS_PER_SEC;
}

static void report(const char *structure, const char *operation,
                   size_t n, size_t round, double seconds, uint64_t checksum) {
    observed_checksum = observed_checksum * UINT64_C(1099511628211) ^ checksum;
    printf("%s,%s,%zu,%zu,%.9f,%.2f,%" PRIu64 "\n", structure, operation,
           n, round + 1U, seconds, seconds * 1e9 / (double)n, checksum);
}

static size_t hash_id(const void *key) {
    return hashmap_hash_string((const char *)key);
}

static int compare_id(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static int compare_uint32(const void *a, const void *b) {
    uint32_t x = *(const uint32_t *)a;
    uint32_t y = *(const uint32_t *)b;
    return (x > y) - (x < y);
}

static void run_hashmap(workload_t *w, size_t round) {
    hashmap_base_t *map = hashmap_base_new(hash_id, compare_id);
    require(map != NULL, "hashmap creation failed");
    uint64_t sum = 0;
    clock_t start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        require(hashmap_base_put(map, w->ids[idx], &w->values[idx]) == 0,
                "hashmap insertion failed");
        sum += w->values[idx];
    }
    report("hashmap", "insert", w->count, round, elapsed(start), sum);
    require(hashmap_base_size(map) == w->count, "hashmap size after insertion");

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        const uint64_t *value = hashmap_base_get(map, w->ids[idx]);
        require(value != NULL && *value == w->values[idx], "hashmap lookup mismatch");
        sum += *value;
    }
    report("hashmap", "lookup", w->count, round, elapsed(start), sum);

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        const uint64_t *value = hashmap_base_remove(map, w->ids[idx]);
        require(value == &w->values[idx], "hashmap removal mismatch");
        sum += *value;
    }
    report("hashmap", "remove", w->count, round, elapsed(start), sum);
    require(hashmap_base_size(map) == 0U, "hashmap not emptied");
    hashmap_base_destroy(map);
}

static void run_bostree(workload_t *w, size_t round) {
    bostree_t *tree = bostree_new(compare_uint32, NULL);
    require(tree != NULL, "tree creation failed");
    uint64_t sum = 0;
    clock_t start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        w->nodes[idx] = bostree_insert(tree, &w->keys[idx], &w->values[idx]);
        require(w->nodes[idx] != NULL, "tree insertion failed");
        sum += (uint64_t)idx;
    }
    report("bostree", "insert", w->count, round, elapsed(start), sum);
    require(bostree_node_count(tree) == w->count, "tree size after insertion");

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        bostree_node_t *node = bostree_lookup(tree, &w->keys[idx]);
        require(node == w->nodes[idx], "tree lookup mismatch");
        sum += (uint64_t)idx;
    }
    report("bostree", "lookup", w->count, round, elapsed(start), sum);

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        bostree_node_t *node = bostree_select(tree, (unsigned int)i);
        require(node != NULL && *(const uint32_t *)bostree_node_key(node) == i,
                "tree select mismatch");
        require(bostree_rank(node) == i, "tree rank mismatch");
        sum += (uint64_t)i;
    }
    report("bostree", "select_rank", w->count, round, elapsed(start), sum);

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        bostree_remove(tree, w->nodes[idx]);
        w->nodes[idx] = NULL;
        sum += (uint64_t)idx;
    }
    report("bostree", "remove", w->count, round, elapsed(start), sum);
    require(bostree_node_count(tree) == 0U, "tree not emptied");
    bostree_destroy(tree);
}

static void run_ranking(workload_t *w, size_t round) {
    ranking_t *ranking = ranking_new();
    require(ranking != NULL, "ranking creation failed");
    uint64_t sum = 0;
    clock_t start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        require(ranking_upsert(ranking, w->ids[idx], w->scores[idx]) == 0,
                "ranking insertion failed");
        sum += (uint64_t)w->scores[idx];
    }
    report("ranking", "insert", w->count, round, elapsed(start), sum);
    require(ranking_size(ranking) == w->count, "ranking size after insertion");

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        int64_t score = 0;
        size_t position = 0;
        require(ranking_get(ranking, w->ids[idx], &score, &position) == 0 &&
                score == w->scores[idx] && position < w->count,
                "ranking lookup mismatch");
        sum += (uint64_t)score + (uint64_t)position;
    }
    report("ranking", "lookup_rank", w->count, round, elapsed(start), sum);

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        const char *id = NULL;
        int64_t score = 0;
        require(ranking_at(ranking, i, &id, &score) == 0 && id != NULL,
                "ranking select failed");
        sum += (uint64_t)score + (unsigned char)id[0];
    }
    report("ranking", "select", w->count, round, elapsed(start), sum);

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        w->scores[idx] += INT64_C(100000);
        require(ranking_upsert(ranking, w->ids[idx], w->scores[idx]) == 0,
                "ranking score update failed");
        sum += (uint64_t)w->scores[idx];
    }
    report("ranking", "update", w->count, round, elapsed(start), sum);
    require(ranking_size(ranking) == w->count, "ranking size after update");

    sum = 0;
    start = clock();
    for (size_t i = 0; i < w->count; ++i) {
        size_t idx = w->permutation[i];
        require(ranking_remove(ranking, w->ids[idx]) == 0, "ranking removal failed");
        sum += (uint64_t)idx;
    }
    report("ranking", "remove", w->count, round, elapsed(start), sum);
    require(ranking_size(ranking) == 0U, "ranking not emptied");
    ranking_destroy(ranking);
}

int main(int argc, char **argv) {
    if (argc > 3) {
        fprintf(stderr, "usage: %s [entries:1..%u] [rounds:1..%u]\n",
                argv[0], MAX_ENTRIES, MAX_ROUNDS);
        return EXIT_FAILURE;
    }
    size_t count = argc > 1 ? parse_positive(argv[1], MAX_ENTRIES) : 10000U;
    size_t rounds = argc > 2 ? parse_positive(argv[2], MAX_ROUNDS) : 3U;
    workload_t w = {0};
    w.ids = calloc(count, sizeof(*w.ids));
    w.permutation = calloc(count, sizeof(*w.permutation));
    w.keys = calloc(count, sizeof(*w.keys));
    w.values = calloc(count, sizeof(*w.values));
    w.scores = calloc(count, sizeof(*w.scores));
    w.nodes = calloc(count, sizeof(*w.nodes));
    require(w.ids && w.permutation && w.keys && w.values && w.scores && w.nodes,
            "could not allocate benchmark fixtures");
    puts("structure,operation,count,iteration,cpu_seconds,ns_per_op,checksum");
    for (size_t round = 0; round < rounds; ++round) {
        prepare(&w, count, round);
        run_hashmap(&w, round);
        run_bostree(&w, round);
        run_ranking(&w, round);
    }
    fprintf(stderr, "benchmark completed: entries=%zu, rounds=%zu, sink=%" PRIu64 "\n",
            count, rounds, observed_checksum);
    free(w.ids);
    free(w.permutation);
    free(w.keys);
    free(w.values);
    free(w.scores);
    free(w.nodes);
    return EXIT_SUCCESS;
}
