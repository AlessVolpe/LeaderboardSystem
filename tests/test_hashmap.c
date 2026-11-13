/*
 * LeaderboardSystem hashmap smoke tests (milestone 3.0.1).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #cond); exit(EXIT_FAILURE); } } while (0)
#include <string.h>

#include "hashmap.h"

int main(void) {
    HASHMAP(char, int)
    map;
    int value = 42;

    CHECK(hashmap_init(&map, hashmap_hash_string, strcmp) == 0);

    CHECK(hashmap_put(&map, "alpha", &value) == 0);
    CHECK(hashmap_size(&map) == 1);
    CHECK(hashmap_get(&map, "alpha") == &value);
    CHECK(hashmap_remove(&map, "alpha") == &value);
    CHECK(hashmap_get(&map, "alpha") == NULL);
    CHECK(hashmap_size(&map) == 0);

    hashmap_cleanup(&map);
    return 0;
}
