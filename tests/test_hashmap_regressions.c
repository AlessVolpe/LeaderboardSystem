/*
 * LeaderboardSystem hashmap regressions (milestone 3.0.2).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap_base.h"

/* CHECK must remain active in Release builds (assert disappears under NDEBUG). */
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static size_t hash_string(const void *key) {
    return hashmap_hash_string((const char *) key);
}

static int cmp_string(const void *lhs, const void *rhs) {
    return strcmp((const char *) lhs, (const char *) rhs);
}

static size_t constant_hash(const void *key) {
    (void) key;
    return 7u;
}

static unsigned copies;
static unsigned releases;
static int fail_copy;

static void *clone_key(const void *key) {
    if (fail_copy) return NULL;
    size_t len = strlen((const char *) key) + 1u;
    char *copy = malloc(len);
    if (copy == NULL) return NULL;
    memcpy(copy, key, len);
    ++copies;
    return copy;
}

static void release_key(void *key) {
    ++releases;
    free(key);
}

static void test_empty_and_invalid_inputs(void) {
    hashmap_base m;
    hashmap_base_init(&m, hash_string, cmp_string);
    int data = 1;
    CHECK(hashmap_base_get(&m, "x") == NULL);
    CHECK(hashmap_base_remove(&m, "x") == NULL);
    CHECK(hashmap_base_iter(&m, NULL) == NULL);
    CHECK(hashmap_base_load_factor(&m) == 0.0);
    CHECK(hashmap_base_collisions(&m, "x") == 0);
    CHECK(hashmap_base_put(&m, NULL, &data) == -EINVAL);
    CHECK(hashmap_base_put(&m, "x", NULL) == -EINVAL);
    CHECK(hashmap_base_iter_set_data(NULL, &data) == -EFAULT);
    CHECK(hashmap_base_reserve(&m, SIZE_MAX) == -EOVERFLOW);
    CHECK(m.table_size == 0);
    hashmap_base_clear(&m);
    hashmap_base_reset(&m);
    CHECK(m.table == NULL);
    hashmap_base_cleanup(&m);
    hashmap_base_cleanup(&m);
}

static void test_key_ownership_and_duplicates(void) {
    hashmap_base m;
    hashmap_base_init(&m, hash_string, cmp_string);
    copies = releases = 0;
    fail_copy = 0;
    int one = 1, two = 2;
    CHECK(hashmap_base_set_key_alloc_funcs(&m, clone_key, NULL) == -EINVAL);
    CHECK(hashmap_base_set_key_alloc_funcs(&m, NULL, release_key) == -EINVAL);
    CHECK(hashmap_base_set_key_alloc_funcs(&m, clone_key, release_key) == 0);

    char temporary[] = "alpha";
    CHECK(hashmap_base_put(&m, temporary, &one) == 0);
    strcpy(temporary, "other");
    CHECK(hashmap_base_get(&m, "alpha") == &one);
    CHECK(hashmap_base_get(&m, "other") == NULL);
    CHECK(copies == 1 && releases == 0);
    CHECK(hashmap_base_put(&m, "alpha", &two) == -EEXIST);
    CHECK(hashmap_base_get(&m, "alpha") == &one);
    CHECK(copies == 1);

    CHECK(hashmap_base_set_key_alloc_funcs(&m, NULL, NULL) == -EBUSY);
    CHECK(hashmap_base_set_key_alloc_funcs(&m, clone_key, release_key) == -EBUSY);
    fail_copy = 1;
    CHECK(hashmap_base_put(&m, "beta", &two) == -ENOMEM);
    CHECK(m.size == 1);
    fail_copy = 0;
    CHECK(hashmap_base_put(&m, "beta", &two) == 0);
    CHECK(copies == 2);
    CHECK(hashmap_base_remove(&m, "alpha") == &one);
    CHECK(releases == 1);
    CHECK(hashmap_base_get(&m, "beta") == &two);
    hashmap_base_clear(&m);
    CHECK(releases == 2 && m.size == 0);
    CHECK(hashmap_base_set_key_alloc_funcs(&m, NULL, NULL) == 0);
    CHECK(hashmap_base_put(&m, "borrowed", &one) == 0);
    hashmap_base_cleanup(&m);
    CHECK(releases == 2); /* Neither borrowed keys nor values are ever freed. */
}

static void test_owned_keys_survive_rehash(void) {
    hashmap_base m;
    hashmap_base_init(&m, hash_string, cmp_string);
    copies = releases = 0;
    fail_copy = 0;
    CHECK(hashmap_base_set_key_alloc_funcs(&m, clone_key, release_key) == 0);
    int values[200];
    for (int i = 0; i < 200; ++i) {
        char temporary[32];
        snprintf(temporary, sizeof(temporary), "owned-%03d", i);
        values[i] = i;
        CHECK(hashmap_base_put(&m, temporary, &values[i]) == 0);
    }
    CHECK(m.table_size > 128);
    CHECK(copies == 200 && releases == 0);
    for (int i = 0; i < 200; ++i) {
        char key[32];
        snprintf(key, sizeof(key), "owned-%03d", i);
        CHECK(hashmap_base_get(&m, key) == &values[i]);
    }
    hashmap_base_reset(&m);
    CHECK(releases == 200 && m.size == 0);
    hashmap_base_cleanup(&m);
    CHECK(releases == 200);
}

static void test_collisions_backward_shift_and_iteration(void) {
    enum { N = 256 };
    hashmap_base m;
    hashmap_base_init(&m, constant_hash, cmp_string);
    char keys[N][20];
    int values[N];
    for (size_t i = 0; i < N; ++i) {
        snprintf(keys[i], sizeof(keys[i]), "key_%04zu", i);
        values[i] = (int) i;
        CHECK(hashmap_base_put(&m, keys[i], &values[i]) == 0);
        CHECK(hashmap_base_load_factor(&m) <= .75);
    }
    CHECK(m.size == N);
    CHECK(hashmap_base_collisions_mean(&m) > 0);
    CHECK(hashmap_base_collisions_variance(&m) > 0);
    CHECK(hashmap_base_collisions(&m, keys[N - 1]) == N - 1);

    for (size_t i = 0; i < N; i += 2) {
        CHECK(hashmap_base_remove(&m, keys[i]) == &values[i]);
        CHECK(hashmap_base_get(&m, keys[i]) == NULL);
    }
    CHECK(m.size == N / 2);
    for (size_t i = 1; i < N; i += 2)
        CHECK(hashmap_base_get(&m, keys[i]) == &values[i]);

    size_t nonmutating_walk = 0;
    for (hashmap_entry *it = hashmap_base_iter(&m, NULL); it != NULL;
         (void) hashmap_base_iter_next(&m, &it)) {
        CHECK(hashmap_base_iter_valid(&m, it));
        CHECK(hashmap_base_iter_get_key(it) != NULL);
        ++nonmutating_walk;
    }
    CHECK(nonmutating_walk == N / 2);

    unsigned char visited[N] = {0};
    size_t walked = 0;
    for (hashmap_entry *it = hashmap_base_iter(&m, NULL); it != NULL;) {
        int *value = hashmap_base_iter_get_data(it);
        CHECK(value != NULL);
        CHECK(*value >= 0 && *value < N);
        CHECK(visited[*value] == 0);
        visited[*value] = 1;
        ++walked;
        (void) hashmap_base_iter_remove(&m, &it);
    }
    CHECK(walked == N / 2);
    CHECK(m.size == 0);
    CHECK(hashmap_base_iter(&m, NULL) == NULL);
    for (size_t i = 1; i < N; i += 2)
        CHECK(visited[i] == 1);
    hashmap_base_cleanup(&m);
}

static void test_reserve_reset_and_setter(void) {
    hashmap_base m;
    hashmap_base_init(&m, hash_string, cmp_string);
    CHECK(hashmap_base_reserve(&m, 15) == 0);
    CHECK(m.table_size_init == 32);
    CHECK(m.table_size >= 32);
    char keys[300][20];
    int values[300];
    for (int i = 0; i < 300; ++i) {
        snprintf(keys[i], sizeof(keys[i]), "id-%d", i);
        values[i] = i;
        CHECK(hashmap_base_put(&m, keys[i], &values[i]) == 0);
    }
    CHECK(m.table_size > m.table_size_init);
    hashmap_entry *it = hashmap_base_iter(&m, NULL);
    CHECK(it != NULL && hashmap_base_iter_valid(&m, it));
    int replacement = 999;
    CHECK(hashmap_base_iter_set_data(it, NULL) == -EINVAL);
    CHECK(hashmap_base_iter_set_data(it, &replacement) == 0);
    CHECK(hashmap_base_get(&m, hashmap_base_iter_get_key(it)) == &replacement);
    CHECK(!hashmap_base_iter_valid(&m, NULL));
    hashmap_base_reset(&m);
    CHECK(m.size == 0 && m.table_size == 32);
    CHECK(hashmap_base_get(&m, "id-0") == NULL);
    CHECK(hashmap_base_put(&m, "new", &replacement) == 0);
    CHECK(hahsmap_base_remove(&m, "new") == &replacement);
    hashmap_base_cleanup(&m);
}

static uint32_t rng_state = UINT32_C(0x1234abcd);

static uint32_t next_random(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return rng_state = x;
}

static void test_deterministic_differential(void) {
    enum { N = 257, STEPS = 14000 };
    hashmap_base m;
    hashmap_base_init(&m, hash_string, cmp_string);
    char keys[N][20];
    int values[N];
    unsigned char present[N] = {0};
    size_t expected_size = 0;
    for (int i = 0; i < N; ++i) {
        snprintf(keys[i], sizeof(keys[i]), "item-%03d", i);
        values[i] = i + 100;
    }
    for (int step = 0; step < STEPS; ++step) {
        unsigned r = next_random();
        int i = (int) (r % N);
        switch ((r >> 16) % 9) {
            case 0:
            case 1:
            case 2:
                CHECK(hashmap_base_put(&m, keys[i], &values[i]) == (present[i] ? -EEXIST : 0));
                if (!present[i]) {
                    present[i] = 1;
                    ++expected_size;
                }
                break;
            case 3:
            case 4:
                CHECK(hashmap_base_remove(&m, keys[i]) == (present[i] ? &values[i] : NULL));
                if (present[i]) {
                    present[i] = 0;
                    --expected_size;
                }
                break;
            case 5:
                hashmap_base_clear(&m);
                memset(present, 0, sizeof(present));
                expected_size = 0;
                break;
            case 6:
                hashmap_base_reset(&m);
                memset(present, 0, sizeof(present));
                expected_size = 0;
                break;
            case 7:
                CHECK(hashmap_base_reserve(&m, r % N) == 0);
                break;
            default:
                CHECK(hashmap_base_get(&m, keys[i]) == (present[i] ? &values[i] : NULL));
                break;
        }
        CHECK(m.size == expected_size);
        CHECK(hashmap_base_load_factor(&m) <= .75);
        if (step % 37 == 0) {
            size_t seen = 0;
            for (int k = 0; k < N; ++k) {
                CHECK(hashmap_base_get(&m, keys[k]) == (present[k] ? &values[k] : NULL));
                seen += present[k];
            }
            CHECK(seen == expected_size);
        }
    }
    hashmap_base_cleanup(&m);
}

int main(void) {
    test_empty_and_invalid_inputs();
    test_key_ownership_and_duplicates();
    test_owned_keys_survive_rehash();
    test_collisions_backward_shift_and_iteration();
    test_reserve_reset_and_setter();
    test_deterministic_differential();
    puts("hashmap regressions: passed");
    return 0;
}
