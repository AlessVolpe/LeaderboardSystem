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
#include "hashmap.h"
#include "mtwister.h"
#include "player.h"

_Static_assert(MAX_NAME_SIZE == 25, "Keep the input width synchronized with MAX_NAME_SIZE");

/* Names are owned by player_names; the tree owns only player_t allocations. */
static void _tree_free(BOSNode *node) {
    free(bostree_node_data(node));
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

    for (size_t i = 0; i < count; ++i) {
        names[i] = malloc(MAX_NAME_SIZE);
        if (names[i] == NULL) {
            _free_player_names(count, names);
            return NULL;
        }
        printf("Insert player name: \n");
        /* MAX_NAME_SIZE is 25, including the terminating NUL. */
        if (scanf("%24s", names[i]) != 1) {
            _free_player_names(count, names);
            return NULL;
        }
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
    HASHMAP(char, player_t) pool;
    if (hashmap_init(&pool, hashmap_hash_string, strcmp) != 0) {
        fprintf(stderr, "Unable to initialize player hashmap.\n");
        _free_player_names(count, names);
        return EXIT_FAILURE;
    }

    BOSTree *leaderboard = bostree_new(_compare_names, _tree_free);
    MTRand *random = seedRand((uint32_t)time(NULL));
    int result = EXIT_FAILURE;
    const char *key;
    player_t *data;
    void *temp;

    if (leaderboard == NULL || random == NULL) {
        fprintf(stderr, "Unable to initialize leaderboard or random generator.\n");
        goto cleanup;
    }

    for (size_t i = 0; i < count; ++i) {
        char *id = uuid_generate();
        player_t *player = player_create(names[i]);
        if (id == NULL || player == NULL) {
            free(id);
            free(player);
            fprintf(stderr, "Unable to allocate player.\n");
            goto cleanup;
        }

        const int status = hashmap_put(&pool, id, player);
        if (status != 0) {
            fprintf(stderr, "Unable to store player %s (error %d).\n", names[i], status);
            free(player);
            free(id);
            goto cleanup;
        }
        ids[i] = id;
        printf("Added player %s [%s].\n", player->name, id);
    }

    printf("\nFormat: player_name[player_id] with score n\n");
    hashmap_foreach(key, data, &pool) {
        player_adding_points(data, (uint32_t)(genRand(random) * 100.0));
        printf("Player: %s[%s] with score %" PRIu32 "\n", data->name, key, data->score);
    }

    /* Removing transfers player ownership from the pool to the tree. */
    hashmap_foreach_key_safe(key, &pool, temp) {
        player_t *player = hashmap_remove(&pool, key);
        if (player == NULL) {
            fprintf(stderr, "Unable to transfer player to leaderboard.\n");
            goto cleanup;
        }
        BOSNode *node = bostree_insert(leaderboard, player->name, player);
        if (node == NULL) {
            free(player);
            fprintf(stderr, "Unable to insert leaderboard node.\n");
            goto cleanup;
        }
        if (bostree_node_data(node) != player) {
            /* Comparator-equal names share a ranking entry. */
            printf("Duplicate player name %s ignored.\n", player->name);
            free(player);
        } else {
            printf("Inserted player %s with %" PRIu32 " points.\n", player->name, player->score);
        }
    }

#ifndef NDEBUG
    bostree_print(leaderboard);
#endif
    result = EXIT_SUCCESS;

cleanup:
    /* These are the values still in the hashmap, not the ones in the tree. */
    hashmap_foreach_key_safe(key, &pool, temp) {
        free(hashmap_remove(&pool, key));
    }
    hashmap_cleanup(&pool);
    bostree_destroy(leaderboard);
    for (size_t i = 0; i < count; ++i) free(ids[i]);
    _free_player_names(count, names);
    free(random);
    return result;
}
