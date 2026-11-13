/*
 * LeaderboardSystem ranking API tests (milestone 3.0.5).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ranking.h"

#define CHECK(expr) do {                                                       \
    if (!(expr)) {                                                            \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr);            \
        exit(EXIT_FAILURE);                                                   \
    }                                                                         \
} while (0)

static void check_at(ranking_t *ranking, size_t rank,
                     const char *expected_id, int64_t expected_score) {
    const char *id = NULL;
    int64_t score = 0;
    CHECK(ranking_at(ranking, rank, &id, &score) == 0);
    CHECK(strcmp(id, expected_id) == 0);
    CHECK(score == expected_score);
    size_t actual_rank = SIZE_MAX;
    int64_t actual_score = 0;
    CHECK(ranking_get(ranking, id, &actual_score, &actual_rank) == 0);
    CHECK(actual_rank == rank);
    CHECK(actual_score == score);
}

static void test_invalid_and_empty(void) {
    CHECK(ranking_size(NULL) == 0);
    ranking_destroy(NULL);
    CHECK(ranking_upsert(NULL, "id", 0) == -EINVAL);
    CHECK(ranking_remove(NULL, "id") == -EINVAL);
    CHECK(ranking_get(NULL, "id", NULL, NULL) == -EINVAL);
    CHECK(ranking_at(NULL, 0, NULL, NULL) == -EINVAL);

    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    CHECK(ranking_size(ranking) == 0);
    CHECK(ranking_at(ranking, 0, NULL, NULL) == -ENOENT);
    CHECK(ranking_remove(ranking, "nobody") == -ENOENT);
    CHECK(ranking_get(ranking, "nobody", NULL, NULL) == -ENOENT);
    CHECK(ranking_upsert(ranking, NULL, 1) == -EINVAL);
    CHECK(ranking_upsert(ranking, "", 1) == -EINVAL);
    CHECK(ranking_get(ranking, NULL, NULL, NULL) == -EINVAL);
    CHECK(ranking_get(ranking, "", NULL, NULL) == -EINVAL);
    CHECK(ranking_remove(ranking, NULL) == -EINVAL);
    CHECK(ranking_remove(ranking, "") == -EINVAL);
    ranking_destroy(ranking);
}

static void test_ties_and_updates(void) {
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    CHECK(ranking_upsert(ranking, "zulu", 9) == 0);
    CHECK(ranking_upsert(ranking, "alpha", 9) == 0);
    CHECK(ranking_upsert(ranking, "beta", 10) == 0);
    CHECK(ranking_upsert(ranking, "delta", 9) == 0);
    CHECK(ranking_size(ranking) == 4);

    check_at(ranking, 0, "beta", 10);
    check_at(ranking, 1, "alpha", 9);
    check_at(ranking, 2, "delta", 9);
    check_at(ranking, 3, "zulu", 9);
    CHECK(ranking_at(ranking, 4, NULL, NULL) == -ENOENT);
    CHECK(ranking_at(ranking, SIZE_MAX, NULL, NULL) == -ENOENT);
    CHECK(ranking_get(ranking, "alpha", NULL, NULL) == 0);
    CHECK(ranking_at(ranking, 2, NULL, NULL) == 0);

    const char *borrowed = NULL;
    CHECK(ranking_at(ranking, 1, &borrowed, NULL) == 0);
    char *old_id = (char *)borrowed; /* Test pointer identity, not ownership. */
    CHECK(ranking_upsert(ranking, "alpha", 11) == 0);
    check_at(ranking, 0, "alpha", 11);
    CHECK(ranking_size(ranking) == 4);
    CHECK(ranking_at(ranking, 0, &borrowed, NULL) == 0);
    CHECK(borrowed == old_id); /* Borrowed IDs survive score updates. */

    CHECK(ranking_upsert(ranking, "alpha", 11) == 0); /* No-op update. */
    CHECK(ranking_upsert(ranking, "beta", INT64_MIN) == 0);
    CHECK(ranking_upsert(ranking, "zulu", INT64_MAX) == 0);
    check_at(ranking, 0, "zulu", INT64_MAX);
    check_at(ranking, 1, "alpha", 11);
    check_at(ranking, 2, "delta", 9);
    check_at(ranking, 3, "beta", INT64_MIN);

    CHECK(ranking_remove(ranking, "alpha") == 0);
    CHECK(ranking_size(ranking) == 3);
    CHECK(ranking_get(ranking, "alpha", NULL, NULL) == -ENOENT);
    CHECK(ranking_remove(ranking, "alpha") == -ENOENT);
    CHECK(ranking_upsert(ranking, "alpha", INT64_MIN) == 0);
    check_at(ranking, 2, "alpha", INT64_MIN); /* Tie broken by ID. */
    check_at(ranking, 3, "beta", INT64_MIN);
    ranking_destroy(ranking);
}

static void test_copies_and_rehash(void) {
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    char id[32] = "temporary";
    CHECK(ranking_upsert(ranking, id, -2) == 0);
    strcpy(id, "overwritten");
    CHECK(ranking_get(ranking, "temporary", NULL, NULL) == 0);
    CHECK(ranking_get(ranking, "overwritten", NULL, NULL) == -ENOENT);

    for (int i = 0; i < 1024; ++i) {
        char key[32];
        (void)snprintf(key, sizeof(key), "entry-%04d", i);
        CHECK(ranking_upsert(ranking, key, (int64_t)(i % 7 - 3)) == 0);
    }
    CHECK(ranking_size(ranking) == 1025);
    for (int i = 0; i < 1024; ++i) {
        char key[32];
        (void)snprintf(key, sizeof(key), "entry-%04d", i);
        CHECK(ranking_get(ranking, key, NULL, NULL) == 0);
    }
    CHECK(ranking_upsert(ranking, "temporary", INT64_MAX) == 0);
    check_at(ranking, 0, "temporary", INT64_MAX);
    for (int i = 0; i < 1024; i += 2) {
        char key[32];
        (void)snprintf(key, sizeof(key), "entry-%04d", i);
        CHECK(ranking_remove(ranking, key) == 0);
    }
    CHECK(ranking_size(ranking) == 513);
    for (int i = 0; i < 1024; ++i) {
        char key[32];
        (void)snprintf(key, sizeof(key), "entry-%04d", i);
        CHECK(ranking_get(ranking, key, NULL, NULL) ==
              (i % 2 == 0 ? -ENOENT : 0));
    }
    ranking_destroy(ranking);
}

int main(void) {
    test_invalid_and_empty();
    test_ties_and_updates();
    test_copies_and_rehash();
    puts("ranking regressions: passed");
    return 0;
}
