/*
 * LeaderboardSystem installed-package consumer (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ranking.h"
#include "mtwister.h"
#include "player.h"
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
    if (!ok) return EXIT_FAILURE;

    /* Test that the installed headers expose modern names and the exported
     * library provides the standards-compatible MT19937 entry points. */
    mtwister_t *rng = mtwister_new(UINT32_C(5489));
    if (rng == NULL) return EXIT_FAILURE;
    ok = mtwister_next_u32(rng) == UINT32_C(3499211612) &&
         mtwister_next_u32(rng) == UINT32_C(581869302);
    mtwister_destroy(rng);
    if (!ok) return EXIT_FAILURE;

    char team_name[] = "Brigate Ebosse";
    player_t *team = player_create_with_total(team_name, UINT32_C(24835));
    if (team == NULL) return EXIT_FAILURE;
    ok = team->championship_points == 0U &&
         team->total_points_tenths == UINT32_C(24835) &&
         player_add_championship_points(team, 3U) == 0 &&
         team->championship_points == 3U;
    player_reset_championship_points(team);
    ok = ok && team->championship_points == 0U &&
         team->total_points_tenths == UINT32_C(24835);
    player_destroy(team);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
