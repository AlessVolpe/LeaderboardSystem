/*
 * LeaderboardSystem team/player model.
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "player.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

player_t *player_create_with_total(char *name, uint32_t total_points_tenths) {
    if (name == NULL || name[0] == '\0') return NULL;
    player_t *player = malloc(sizeof(*player));
    if (player == NULL) return NULL;
    player->name = name;
    player->championship_points = 0U;
    player->total_points_tenths = total_points_tenths;
    return player;
}

player_t *player_create(char *name) {
    return player_create_with_total(name, 0U);
}

void player_destroy(player_t *player) {
    free(player);
}

int player_add_championship_points(player_t *player, uint32_t points) {
    if (player == NULL) return -EINVAL;
    if (UINT32_MAX - player->championship_points < points) return -EOVERFLOW;
    player->championship_points += points;
    return 0;
}

void player_reset_championship_points(player_t *player) {
    if (player != NULL) player->championship_points = 0U;
}

void player_adding_points(player_t *player, uint32_t points) {
    (void)player_add_championship_points(player, points);
}
