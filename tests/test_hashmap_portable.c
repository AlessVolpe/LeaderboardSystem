/*
 * LeaderboardSystem C11 portable hashmap contract tests (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap_typed.h"

HASHMAP_TYPED_DEFINE(score_map, char, int)

#define CHECK(c) do { if (!(c)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); \
    return EXIT_FAILURE; } } while (0)

static unsigned copies;
static unsigned releases;
static char *copy_key(const char *s) {
    size_t n = strlen(s) + 1;
    char *dest = malloc(n);
    if (dest != NULL) { memcpy(dest, s, n); ++copies; }
    return dest;
}
static void free_key(char *s) { ++releases; free(s); }

int main(void) {
    score_map_t map = {0};
    CHECK(score_map_init(&map, hashmap_hash_string, strcmp) == 0);
    CHECK(score_map_set_key_alloc_funcs(&map, copy_key, free_key) == 0);
    CHECK(score_map_reserve(&map, 256) == 0);
    int one = 1, two = 2, three = 3;
    char mutable_id[] = "alpha";
    CHECK(score_map_put(&map, mutable_id, &one) == 0);
    memcpy(mutable_id, "xxxxx", sizeof mutable_id);
    CHECK(score_map_get(&map, "alpha") == &one);
    CHECK(score_map_get(&map, "xxxxx") == NULL);
    CHECK(score_map_put(&map, "alpha", &two) == -EEXIST);
    CHECK(score_map_put(&map, "beta", &two) == 0);
    CHECK(score_map_put(&map, "gamma", &three) == 0);
    CHECK(score_map_size(&map) == 3);
    CHECK(score_map_set_key_alloc_funcs(&map, copy_key, free_key) == -EBUSY);
    score_map_iterator_t it = score_map_iter(&map);
    size_t visited = 0;
    while (score_map_iter_valid(&it)) {
        CHECK(score_map_iter_key(&it) != NULL);
        CHECK(score_map_iter_value(&it) != NULL);
        ++visited;
        if (strcmp(score_map_iter_key(&it), "beta") == 0)
            (void)score_map_iter_remove(&it);
        else
            (void)score_map_iter_next(&it);
    }
    CHECK(visited == 3);
    CHECK(score_map_size(&map) == 2);
    CHECK(score_map_get(&map, "beta") == NULL);
    CHECK(score_map_remove(&map, "alpha") == &one);
    CHECK(score_map_size(&map) == 1);
    score_map_cleanup(&map);
    CHECK(map.base == NULL);
    CHECK(copies == 3 && releases == 3);

    score_map_t borrowed = {0};
    CHECK(score_map_init(&borrowed, hashmap_hash_string, strcmp) == 0);
    CHECK(score_map_put(&borrowed, "free", &one) == 0);
    CHECK(score_map_get(&borrowed, "free") == &one);
    score_map_cleanup(&borrowed);
    return EXIT_SUCCESS;
}
