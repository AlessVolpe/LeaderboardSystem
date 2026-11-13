/*
 * LeaderboardSystem player model (legacy).
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#define MAX_NAME_SIZE   25

typedef struct player {
    char *name;
    uint32_t score;
} player_t;

player_t *player_create(char *name);

void player_adding_points(player_t *player, uint32_t points);

#endif //PLAYER_H
