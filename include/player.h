/*
 * LeaderboardSystem team/player model.
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_SIZE   25

/* A team across a season and one calendar simulation.
 * name is BORROWED: the caller must keep the NUL-terminated string alive
 * for as long as this player exists, and free it independently if owned.
 * championship_points is simulation-dependent and starts at zero.
 * total_points_tenths is a nonnegative, fixed-point season aggregate (x10)
 * independent of fixture permutations. 2483.5 is stored as 24835.
 * The struct is public; changing it requires clients to be recompiled. */
typedef struct player {
    char *name;
    uint32_t championship_points;
    uint32_t total_points_tenths;
} player_t;

/* Return NULL for NULL/empty name or allocation failure.
 * Historical constructor: initializes both numeric fields to zero. */
player_t *player_create(char *name);

/* Initialize the season aggregate once while keeping championship points zero. */
player_t *player_create_with_total(char *name, uint32_t total_points_tenths);

/* Frees only the player_t allocation, NOT its borrowed name. NULL is safe. */
void player_destroy(player_t *player);

/* Add league standings points, typically 3/1/0 for W/D/L. Returns 0 on
 * success, -EINVAL for NULL, -EOVERFLOW if uint32_t would wrap. On failure
 * both point fields remain unchanged. The caller enforces match rules. */
int player_add_championship_points(player_t *player, uint32_t points);

/* Reset only the calendar-dependent championship points. NULL is safe. */
void player_reset_championship_points(player_t *player);

/* Historical spelling preserved for source compatibility. It adds to
 * championship_points, and silently ignores errors (including overflow).
 * Prefer player_add_championship_points() for checked arithmetic. */
void player_adding_points(player_t *player, uint32_t points);

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_H */
