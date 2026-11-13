/*
 * LeaderboardSystem public BOSTree API tests (milestone 3.0.6).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "bostree.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
    exit(EXIT_FAILURE); \
} } while (0)

static unsigned int frees;
static int last_finalized_key;
static int last_finalized_data;

static int compare_int(const void *lhs, const void *rhs) {
    int a = *(const int *)lhs, b = *(const int *)rhs;
    return (a > b) - (a < b);
}

/* Even though disposal marks the node invalid first, its payload pointers
 * must remain readable until the finalizer returns. */
static void finalize(bostree_node_t *node) {
    CHECK(!bostree_node_is_valid(node));
    const int *key = bostree_node_key(node);
    int *data = bostree_node_data(node);
    CHECK(key != NULL && data != NULL);
    last_finalized_key = *key;
    last_finalized_data = *data;
    ++frees;
}

int main(void) {
    int keys[] = {30, 10, 40, 20};
    int values[] = {300, 100, 400, 200};
    CHECK(bostree_node_key(NULL) == NULL);
    CHECK(bostree_node_data(NULL) == NULL);
    CHECK(!bostree_node_is_valid(NULL));

    bostree_t *tree = bostree_new(compare_int, finalize);
    CHECK(tree != NULL);
    for (unsigned int i = 0; i < 4; ++i) {
        bostree_node_t *node = bostree_insert(tree, &keys[i], &values[i]);
        CHECK(node != NULL);
        CHECK(bostree_node_key(node) == &keys[i]);
        CHECK(bostree_node_data(node) == &values[i]);
        CHECK(bostree_node_is_valid(node));
    }
    CHECK(bostree_node_count(tree) == 4U);
    CHECK(bostree_select(tree, 4U) == NULL);

    BOSNode *node = bostree_lookup(tree, &keys[0]); /* old typedef works */
    CHECK(node != NULL && bostree_node_weak_ref(node) == node);
    CHECK(bostree_rank(node) == 2U);
    CHECK(bostree_node_key(bostree_select(tree, 0U)) == &keys[1]);
    bostree_remove(tree, node);
    CHECK(!bostree_node_is_valid(node));
    CHECK(bostree_node_key(node) == NULL);
    CHECK(bostree_node_data(node) == NULL);
    CHECK(bostree_rank(node) == UINT_MAX);
    CHECK(frees == 1U && last_finalized_key == 30 && last_finalized_data == 300);
    CHECK(bostree_node_weak_unref(NULL, node) == NULL);
    CHECK(bostree_node_count(tree) == 3U);

    BOSTree *old_alias = tree; /* old tree typedef works */
    bostree_destroy(old_alias);
    CHECK(frees == 4U);
    return EXIT_SUCCESS;
}
