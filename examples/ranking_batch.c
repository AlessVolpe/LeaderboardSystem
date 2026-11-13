/*
 * LeaderboardSystem deterministic, non-interactive ranking example (3.0.9).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * A public-API-only example suitable for CI, batch jobs and documentation.
 * Run: ./leaderboard_ranking_batch
 */
#include "ranking.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *id;
    int64_t score;
} sample_entry_t;

int main(void) {
    const sample_entry_t initial[] = {
        {"grace", 93}, {"ada", 80}, {"linus", 80},
        {"dennis", 88}, {"margaret", -5}
    };
    const sample_entry_t expected[] = {
        {"ada", 95}, {"grace", 93}, {"dennis", 88},
        {"linus", 80}, {"margaret", -5}
    };
    ranking_t *ranking = ranking_new();
    if (ranking == NULL) {
        fputs("ranking_new: out of memory\n", stderr);
        return EXIT_FAILURE;
    }

    int ok = 1;
    for (size_t i = 0; i < sizeof initial / sizeof initial[0]; ++i) {
        if (ranking_upsert(ranking, initial[i].id, initial[i].score) != 0) {
            ok = 0;
            break;
        }
    }
    /* The update is atomic and changes the expected order. */
    if (ok && ranking_upsert(ranking, "ada", 95) != 0) ok = 0;
    if (ok && ranking_size(ranking) != sizeof expected / sizeof expected[0]) ok = 0;

    for (size_t rank = 0; ok && rank < ranking_size(ranking); ++rank) {
        const char *id = NULL;
        int64_t score = 0;
        size_t roundtrip_rank = SIZE_MAX;
        if (ranking_at(ranking, rank, &id, &score) != 0 ||
            id == NULL || strcmp(id, expected[rank].id) != 0 ||
            score != expected[rank].score ||
            ranking_get(ranking, id, NULL, &roundtrip_rank) != 0 ||
            roundtrip_rank != rank) {
            ok = 0;
            break;
        }
        printf("%zu. %s: %" PRId64 "\n", rank + 1U, id, score);
    }

    /* Removal changes later ranks and releases the copied identifier. */
    if (ok && (ranking_remove(ranking, "linus") != 0 ||
               ranking_size(ranking) != 4U)) ok = 0;
    if (ok) {
        size_t rank = SIZE_MAX;
        if (ranking_get(ranking, "margaret", NULL, &rank) != 0 || rank != 3U)
            ok = 0;
    }
    ranking_destroy(ranking);
    if (!ok) {
        fputs("ranking batch example: invariant check failed\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
