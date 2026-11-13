/*
 * LeaderboardSystem installed-package consumer (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ranking.h"
#include "hashmap_typed.h"

HASHMAP_TYPED_DEFINE(external_score_map, char, int)

int main(void) {
    ranking_t *ranking = ranking_new();
    if (ranking == NULL) return EXIT_FAILURE;
    int64_t score = 0;
    size_t position = 0;
    int ok = ranking_upsert(ranking, "z", 12) == 0 &&
             ranking_upsert(ranking, "a", 12) == 0 &&
             ranking_get(ranking, "a", &score, &position) == 0 &&
             score == 12 && position == 0;
    ranking_destroy(ranking);
    if (!ok) return EXIT_FAILURE;

    external_score_map_t map = {0};
    if (external_score_map_init(&map, hashmap_hash_string, strcmp) != 0)
        return EXIT_FAILURE;
    int points = 25;
    ok = external_score_map_put(&map, "participant", &points) == 0 &&
         external_score_map_get(&map, "participant") == &points;
    external_score_map_cleanup(&map);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
