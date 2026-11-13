/*
 * LeaderboardSystem ranking differential tests (milestone 3.0.5).
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

enum { ITEMS = 193, STEPS = 15000 };

static char keys[ITEMS][24];
static int64_t scores[ITEMS];
static unsigned char present[ITEMS];
static size_t model_count;

static uint32_t next_random(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}

static int model_compare(const void *lhs, const void *rhs) {
    const unsigned int a = *(const unsigned int *)lhs;
    const unsigned int b = *(const unsigned int *)rhs;
    if (scores[a] > scores[b]) return -1;
    if (scores[a] < scores[b]) return 1;
    return strcmp(keys[a], keys[b]);
}

static void verify_all(const ranking_t *ranking) {
    unsigned int order[ITEMS];
    size_t count = 0;
    for (unsigned int i = 0; i < ITEMS; ++i) {
        if (present[i]) order[count++] = i;
    }
    CHECK(count == model_count);
    CHECK(ranking_size(ranking) == count);
    qsort(order, count, sizeof(order[0]), model_compare);

    CHECK(ranking_at(ranking, count, NULL, NULL) == -ENOENT);
    for (size_t rank = 0; rank < count; ++rank) {
        const char *id = NULL;
        int64_t actual_score = 0;
        CHECK(ranking_at(ranking, rank, &id, &actual_score) == 0);
        CHECK(strcmp(id, keys[order[rank]]) == 0);
        CHECK(actual_score == scores[order[rank]]);
        size_t actual_rank = SIZE_MAX;
        CHECK(ranking_get(ranking, id, NULL, &actual_rank) == 0);
        CHECK(actual_rank == rank);
    }
    for (unsigned int i = 0; i < ITEMS; ++i) {
        int64_t score = 0;
        CHECK(ranking_get(ranking, keys[i], &score, NULL) ==
              (present[i] ? 0 : -ENOENT));
        if (present[i]) CHECK(score == scores[i]);
    }
}

int main(void) {
    ranking_t *ranking = ranking_new();
    CHECK(ranking != NULL);
    for (unsigned int i = 0; i < ITEMS; ++i)
        (void)snprintf(keys[i], sizeof(keys[i]), "participant-%03u", i);

    uint32_t random = UINT32_C(0x6D305A17);
    for (int step = 0; step < STEPS; ++step) {
        const uint32_t r = next_random(&random);
        const unsigned int index = (unsigned int)(r % ITEMS);
        const unsigned int op = (r >> 16) % 10U;
        if (op <= 5U) {
            int64_t score = (int64_t)(next_random(&random) % 25U) - 12;
            if (op == 0U) score = INT64_MIN;
            if (op == 1U) score = INT64_MAX;
            if (op == 2U && present[index]) score = scores[index];
            CHECK(ranking_upsert(ranking, keys[index], score) == 0);
            if (!present[index]) {
                present[index] = 1U;
                ++model_count;
            }
            scores[index] = score;
        } else if (op <= 7U) {
            CHECK(ranking_remove(ranking, keys[index]) ==
                  (present[index] ? 0 : -ENOENT));
            if (present[index]) {
                present[index] = 0U;
                --model_count;
            }
        } else {
            CHECK(ranking_get(ranking, keys[index], NULL, NULL) ==
                  (present[index] ? 0 : -ENOENT));
        }
        if (step % 23 == 0) verify_all(ranking);
    }
    verify_all(ranking);

    /* Drain in an order unrelated to score and verify each removal. */
    for (unsigned int i = 0; i < ITEMS; ++i) {
        if (!present[i]) continue;
        CHECK(ranking_remove(ranking, keys[i]) == 0);
        present[i] = 0U;
        --model_count;
        if (i % 11U == 0U) verify_all(ranking);
    }
    verify_all(ranking);
    ranking_destroy(ranking);
    puts("ranking differential: passed");
    return 0;
}
