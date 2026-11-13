/*
 * LeaderboardSystem BOSTree private representation (milestone 3.0.6).
 * Copyright (c) 2017, Phillip Berndt
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 * Based on https://github.com/phillipberndt/bostree (GPLv3 or later).
 * Internal only: never install or include from application code.
 */

#ifndef LEADERBOARD_BOSTREE_INTERNAL_H
#define LEADERBOARD_BOSTREE_INTERNAL_H

#include "bostree.h"

typedef struct bostree_node {
    unsigned int left_child_count;
    unsigned int right_child_count;
    unsigned int depth;

    struct bostree_node *left_child_node;
    struct bostree_node *right_child_node;
    struct bostree_node *parent_node;

    void *key;
    void *data;

    unsigned char weak_ref_count;
    unsigned char weak_ref_node_valid;
} bostree_node_t;

typedef struct bostree {
    bostree_node_t *root_node;
    bostree_compare_fn_t cmp_func;
    bostree_free_fn_t free_func;
} bostree_t;

#endif /* LEADERBOARD_BOSTREE_INTERNAL_H */
