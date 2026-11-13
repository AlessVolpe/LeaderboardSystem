/*
 * LeaderboardSystem malformed-input contract regressions (milestone 3.0.8).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bostree.h"
#include "hashmap_base.h"
#include "ranking.h"

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #test); \
    abort(); } } while (0)

static size_t hash_key(const void *key) {
    return hashmap_hash_string((const char *)key);
}
static int compare_key(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}
static void release_key(void *key) { free(key); }
static int compare_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    CHECK(ranking_size(NULL) == 0);
    CHECK(ranking_upsert(NULL, "id", 1) == -EINVAL);
    CHECK(ranking_remove(NULL, "id") == -EINVAL);
    CHECK(ranking_get(NULL, "id", NULL, NULL) == -EINVAL);
    CHECK(ranking_at(NULL, 0, NULL, NULL) == -EINVAL);
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    CHECK(ranking_upsert(ranking, NULL, 1) == -EINVAL);
    CHECK(ranking_upsert(ranking, "", 1) == -EINVAL);
    CHECK(ranking_remove(ranking, "missing") == -ENOENT);
    CHECK(ranking_get(ranking, "missing", NULL, NULL) == -ENOENT);
    CHECK(ranking_at(ranking, 0, NULL, NULL) == -ENOENT);
    CHECK(ranking_upsert(ranking, "low", INT64_MIN) == 0);
    CHECK(ranking_upsert(ranking, "high", INT64_MAX) == 0);
    size_t rank = SIZE_MAX;
    CHECK(ranking_get(ranking, "high", NULL, &rank) == 0 && rank == 0);
    CHECK(ranking_get(ranking, "low", NULL, &rank) == 0 && rank == 1);
    CHECK(ranking_at(ranking, 2, NULL, NULL) == -ENOENT);
    ranking_destroy(ranking);
    ranking_destroy(NULL);

    CHECK(hashmap_base_new(NULL, compare_key) == NULL);
    CHECK(hashmap_base_new(hash_key, NULL) == NULL);
    CHECK(hashmap_base_set_key_alloc_funcs(NULL, NULL, NULL) == -EINVAL);
    CHECK(hashmap_base_reserve(NULL, 1) == -EINVAL);
    CHECK(hashmap_base_put(NULL, "a", "b") == -EINVAL);
    hashmap_base_t *map = hashmap_base_new(hash_key, compare_key);
    CHECK(map != NULL);
    CHECK(hashmap_base_put(map, NULL, "value") == -EINVAL);
    CHECK(hashmap_base_put(map, "id", NULL) == -EINVAL);
    CHECK(hashmap_base_set_key_alloc_funcs(map, NULL, release_key) == -EINVAL);
    CHECK(hashmap_base_reserve(map, SIZE_MAX) == -EOVERFLOW);
    CHECK(hashmap_base_size(map) == 0);
    CHECK(hashmap_base_get(map, NULL) == NULL);
    CHECK(hashmap_base_remove(map, NULL) == NULL);
    CHECK(hashmap_base_put(map, "one", "one value") == 0);
    CHECK(hashmap_base_set_key_alloc_funcs(map, NULL, NULL) == -EBUSY);
    CHECK(hashmap_base_put(map, "one", "second value") == -EEXIST);
    CHECK(hashmap_base_size(map) == 1);
    hashmap_base_destroy(map);
    hashmap_base_destroy(NULL);

    CHECK(bostree_new(NULL, NULL) == NULL);
    CHECK(bostree_node_count(NULL) == 0);
    CHECK(bostree_lookup(NULL, NULL) == NULL);
    CHECK(bostree_select(NULL, 0) == NULL);
    CHECK(bostree_node_weak_ref(NULL) == NULL);
    CHECK(bostree_node_weak_unref(NULL, NULL) == NULL);
    bostree_t *tree = bostree_new(compare_int, NULL);
    CHECK(tree != NULL);
    int key = 1, value = 2;
    CHECK(bostree_insert(tree, NULL, &value) == NULL);
    CHECK(bostree_lookup(tree, NULL) == NULL);
    CHECK(bostree_insert(tree, &key, &value) != NULL);
    CHECK(bostree_node_count(tree) == 1);
    bostree_remove(tree, NULL);
    CHECK(bostree_node_count(tree) == 1);
    bostree_destroy(tree);
    return EXIT_SUCCESS;
}
