/*
 * LeaderboardSystem stateful hashmap fuzz target (milestone 3.0.8).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 * CTest standalone main or optional Clang libFuzzer entry point.
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap_internal.h"

#define KEYS 32u
#define REQUIRE(test) do { if (!(test)) { \
    fprintf(stderr, "hashmap fuzz invariant: %s (%s:%d)\n", #test, __FILE__, __LINE__); \
    abort(); } } while (0)

static size_t collision_hash(const void *key) { (void)key; return 0x5eu; }
static int compare_string(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static void check_state(hashmap_base_t *map, char keys[KEYS][16],
                        int values[KEYS], const unsigned char expected[KEYS]) {
    size_t count = 0;
    for (size_t i = 0; i < KEYS; ++i) {
        const int *value = hashmap_base_get(map, keys[i]);
        if (expected[i]) {
            REQUIRE(value == &values[i]);
            ++count;
        } else REQUIRE(value == NULL);
    }
    REQUIRE(hashmap_base_size(map) == count);
    if (map->table_size != 0) {
        REQUIRE((map->table_size & (map->table_size - 1u)) == 0);
        REQUIRE(map->size <= map->table_size - map->table_size / 4u);
    }
    size_t iterated = 0;
    for (hashmap_entry_t *it = hashmap_base_iter(map, NULL); it != NULL;
         (void)hashmap_base_iter_next(map, &it)) {
        REQUIRE(hashmap_base_iter_valid(map, it));
        const char *key = hashmap_base_iter_get_key(it);
        const int *value = hashmap_base_iter_get_data(it);
        REQUIRE(key != NULL && value != NULL);
        REQUIRE(hashmap_base_get(map, key) == value);
        ++iterated;
        REQUIRE(iterated <= count);
    }
    REQUIRE(iterated == count);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096u) size = 4096u;
    char keys[KEYS][16];
    int values[KEYS];
    unsigned char expected[KEYS] = {0};
    for (unsigned int i = 0; i < KEYS; ++i) {
        (void)snprintf(keys[i], sizeof(keys[i]), "id-%02u", i);
        values[i] = (int)(100u + i);
    }
    hashmap_base_t *map = hashmap_base_new(collision_hash, compare_string);
    REQUIRE(map != NULL);
    for (size_t p = 0; p + 1 < size; p += 2u) {
        unsigned int k = (unsigned int)data[p] % KEYS;
        unsigned int op = (unsigned int)data[p + 1u] % 8u;
        if (op <= 2u) {
            int rc = hashmap_base_put(map, keys[k], &values[k]);
            REQUIRE(rc == (expected[k] ? -EEXIST : 0));
            expected[k] = 1;
        } else if (op == 3u) {
            int *result = hashmap_base_remove(map, keys[k]);
            REQUIRE(result == (expected[k] ? &values[k] : NULL));
            expected[k] = 0;
        } else if (op == 4u) {
            REQUIRE(hashmap_base_get(map, keys[k]) == (expected[k] ? &values[k] : NULL));
        } else if (op == 5u) {
            hashmap_base_clear(map);
            memset(expected, 0, sizeof(expected));
        } else if (op == 6u) {
            hashmap_base_reset(map);
            memset(expected, 0, sizeof(expected));
        } else {
            /* Delete only the current iterator entry: supported safe mutation. */
            hashmap_entry_t *it = hashmap_base_iter(map, NULL);
            size_t seen = 0;
            while (it != NULL) {
                const char *key = hashmap_base_iter_get_key(it);
                REQUIRE(key != NULL);
                unsigned int idx = KEYS;
                for (unsigned int j = 0; j < KEYS; ++j)
                    if (strcmp(keys[j], key) == 0) { idx = j; break; }
                REQUIRE(idx < KEYS && expected[idx]);
                if ((idx & 1u) == (k & 1u)) {
                    expected[idx] = 0;
                    (void)hashmap_base_iter_remove(map, &it);
                } else (void)hashmap_base_iter_next(map, &it);
                REQUIRE(++seen <= KEYS);
            }
        }
        check_state(map, keys, values, expected);
    }
    hashmap_base_destroy(map);
    return 0;
}

#ifndef LEADERBOARD_LIBFUZZER
static uint32_t next_random(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *state = x;
    return x;
}

int main(void) {
    uint8_t input[2048];
    for (uint32_t seed = 1; seed <= 12u; ++seed) {
        uint32_t state = seed * UINT32_C(0x9e3779b9);
        for (size_t i = 0; i < sizeof(input); ++i)
            input[i] = (uint8_t)next_random(&state);
        REQUIRE(LLVMFuzzerTestOneInput(input, sizeof(input)) == 0);
    }
    /* Adversarial collision-chain growth, removal and reinsertion. */
    for (size_t i = 0; i < sizeof(input); i += 2) {
        input[i] = (uint8_t)((i / 2) % KEYS);
        input[i + 1] = (uint8_t)((i / (2u * KEYS)) % 4u == 0 ? 0 : 3);
    }
    REQUIRE(LLVMFuzzerTestOneInput(input, sizeof(input)) == 0);
    return EXIT_SUCCESS;
}
#endif
