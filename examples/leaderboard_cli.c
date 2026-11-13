/*
 * LeaderboardSystem CLI example (legacy).
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bostree.h"
#include "commons.h"
#include "hashmap_typed.h"
#include "mtwister.h"
#include "player.h"

HASHMAP_TYPED_DEFINE(player_pool, char, player_t)

_Static_assert(MAX_NAME_SIZE == 25, "Keep the input width synchronized with MAX_NAME_SIZE");

/* Names are owned by player_names; the tree owns only player_t allocations. */
static void _tree_free(BOSNode *node) {
    player_destroy(bostree_node_data(node));
}

static int _compare_names(const void *lhs, const void *rhs) {
    return strcmp(lhs, rhs);
}

static int _select_players(void) {
    int count;
    printf("Insert player number: \n");
    if (scanf("%d", &count) != 1 || count < MIN_PLAYERS || count > MAX_PLAYERS) {
        fprintf(stderr, "Expected a player count between %d and %d.\n", MIN_PLAYERS, MAX_PLAYERS);
        return -1;
    }
    return count;
}

static void _free_player_names(size_t count, char **names) {
    if (names == NULL) return;
    for (size_t i = 0; i < count; ++i) free(names[i]);
    free(names);
}

static char **_name_players(size_t count) {
    char **names = calloc(count, sizeof(*names));
    if (names == NULL) return NULL;

    /* scanf() used for the count leaves its newline in stdin. */
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}

    for (size_t i = 0; i < count; ++i) {
        names[i] = malloc(MAX_NAME_SIZE);
        if (names[i] == NULL) {
            _free_player_names(count, names);
            return NULL;
        }
        printf("Insert team name: \n");
        /* fgets preserves spaces in team names (e.g. Brigate Ebosse).
         * Detect and reject overlong lines rather than treating the remainder
         * as the next team's name. */
        char line[MAX_NAME_SIZE + 2U];
        if (fgets(line, sizeof(line), stdin) == NULL) {
            _free_player_names(count, names);
            return NULL;
        }
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        } else if (!feof(stdin)) {
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            _free_player_names(count, names);
            return NULL;
        }
        size_t len = strlen(line);
        if (len == 0U || len >= MAX_NAME_SIZE) {
            _free_player_names(count, names);
            return NULL;
        }
        memcpy(names[i], line, len + 1U);
    }
    return names;
}

int main(void) {
    const int requested = _select_players();
    if (requested < 0) return EXIT_FAILURE;

    const size_t count = (size_t)requested;
    char **names = _name_players(count);
    if (names == NULL) {
        fprintf(stderr, "Unable to read or allocate player names.\n");
        return EXIT_FAILURE;
    }

    /* The hashmap borrows UUID keys; ids[] retains ownership until cleanup. */
    char *ids[MAX_PLAYERS] = {NULL};
    player_pool_t pool = {0};
    if (player_pool_init(&pool, hashmap_hash_string, strcmp) != 0) {
        fprintf(stderr, "Unable to initialize player hashmap.\n");
        _free_player_names(count, names);
        return EXIT_FAILURE;
    }

    BOSTree *leaderboard = bostree_new(_compare_names, _tree_free);
    MTRand *random = seedRand((uint32_t)time(NULL));
    int result = EXIT_FAILURE;

    if (leaderboard == NULL || random == NULL) {
        fprintf(stderr, "Unable to initialize leaderboard or random generator.\n");
        goto cleanup;
    }

    for (size_t i = 0; i < count; ++i) {
        char *id = uuid_generate();
        /* Synthetic season aggregate is fixed for this demo fixture.
         * The real Python orchestrator will supply recorded season totals. */
        player_t *player = player_create_with_total(
            names[i], UINT32_C(20000) + genRandLong(random) % UINT32_C(10000));
        if (id == NULL || player == NULL) {
            free(id);
            player_destroy(player);
            fprintf(stderr, "Unable to allocate player.\n");
            goto cleanup;
        }

        const int status = player_pool_put(&pool, id, player);
        if (status != 0) {
            fprintf(stderr, "Unable to store player %s (error %d).\n", names[i], status);
            player_destroy(player);
            free(id);
            goto cleanup;
        }
        ids[i] = id;
        printf("Added player %s [%s].\n", player->name, id);
    }

    printf("\nFormat: team_name[team_id], championship points, fixed season total\n");
    player_pool_iterator_t it = player_pool_iter(&pool);
    while (player_pool_iter_valid(&it)) {
        const char *key = player_pool_iter_key(&it);
        player_t *data = player_pool_iter_value(&it);
        /* One illustrative match result: win (3), draw (1), loss (0). */
        const uint32_t outcome = genRandLong(random) % 3U;
        const uint32_t award = outcome == 0U ? 3U : (outcome == 1U ? 1U : 0U);
        if (player_add_championship_points(data, award) != 0) {
            fprintf(stderr, "Unable to update championship points.\n");
            goto cleanup;
        }
        printf("Team: %s[%s] with %" PRIu32
               " championship points; season fantasy total %" PRIu32 ".%" PRIu32 "\n",
               data->name, key, data->championship_points,
               data->total_points_tenths / 10U, data->total_points_tenths % 10U);
        (void)player_pool_iter_next(&it);
    }

    /* Removing transfers player ownership from the pool to the tree. */
    it = player_pool_iter(&pool);
    while (player_pool_iter_valid(&it)) {
        player_t *player = player_pool_iter_value(&it);
        (void)player_pool_iter_remove(&it);
        BOSNode *node = bostree_insert(leaderboard, player->name, player);
        if (node == NULL) {
            player_destroy(player);
            fprintf(stderr, "Unable to insert leaderboard node.\n");
            goto cleanup;
        }
        if (bostree_node_data(node) != player) {
            /* Comparator-equal names share a ranking entry. */
            printf("Duplicate player name %s ignored.\n", player->name);
            player_destroy(player);
        } else {
            printf("Inserted player %s with %" PRIu32 " points.\n", player->name, player->championship_points);
        }
    }

#ifndef NDEBUG
    bostree_print(leaderboard);
#endif
    result = EXIT_SUCCESS;

cleanup:
    /* These are the values still in the hashmap, not the ones in the tree. */
    it = player_pool_iter(&pool);
    while (player_pool_iter_valid(&it)) {
        player_t *player = player_pool_iter_value(&it);
        (void)player_pool_iter_remove(&it);
        player_destroy(player);
    }
    player_pool_cleanup(&pool);
    bostree_destroy(leaderboard);
    for (size_t i = 0; i < count; ++i) free(ids[i]);
    _free_player_names(count, names);
    free(random);
    return result;
}
