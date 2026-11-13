/*
 * LeaderboardSystem BOSTree smoke tests (milestone 3.0.1).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "bostree.h"

#define CHECK(condition) do {                                                     \
    if (!(condition)) {                                                          \
        fprintf(stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #condition);  \
        exit(EXIT_FAILURE);                                                      \
    }                                                                            \
} while (0)

static int int_compare(const void *lhs, const void *rhs) {
    const int left = *(const int *) lhs;
    const int right = *(const int *) rhs;
    return (left > right) - (left < right);
}

int main(void) {
    int key = 10;
    int value = 42;

    BOSTree *tree = bostree_new(int_compare, NULL);
    CHECK(tree != NULL);

    BOSNode *node = bostree_insert(tree, &key, &value);
    CHECK(node != NULL);
    CHECK(bostree_node_count(tree) == 1);
    CHECK(bostree_lookup(tree, &key) == node);
    CHECK(bostree_select(tree, 0) == node);
    CHECK(bostree_rank(node) == 0);

    bostree_destroy(tree);
    return 0;
}
