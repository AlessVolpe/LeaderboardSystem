/*
 * LeaderboardSystem stateful AVL/order-statistics fuzz target (milestone 3.0.8).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bostree_internal.h"

#define KEYS 32u
#define REQUIRE(test) do { if (!(test)) { \
    fprintf(stderr, "BOSTree fuzz invariant: %s (%s:%d)\n", #test, __FILE__, __LINE__); \
    abort(); } } while (0)

static unsigned int finalized[KEYS];
static unsigned int inserted[KEYS];
static int key_values[KEYS];

static int compare_int(const void *a, const void *b) {
    int lhs = *(const int *)a, rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}
static void finalize_node(bostree_node_t *node) {
    REQUIRE(!bostree_node_is_valid(node));
    const int *key = bostree_node_key(node);
    const int *value = bostree_node_data(node);
    REQUIRE(key != NULL && value == key);
    REQUIRE(*key >= 0 && *key < (int)KEYS);
    REQUIRE(key == &key_values[*key]);
    unsigned int index = (unsigned int)*key;
    REQUIRE(++finalized[index] <= inserted[index]);
}

typedef struct tree_info {
    unsigned int size;
    int height;
} tree_info_t;

static tree_info_t validate_subtree(const bostree_node_t *node,
                                    const bostree_node_t *parent,
                                    int lower, int upper) {
    if (node == NULL) { tree_info_t empty = {0, -1}; return empty; }
    REQUIRE(node->parent_node == parent);
    REQUIRE(node->weak_ref_node_valid && node->weak_ref_count >= 1u);
    REQUIRE(node->key != NULL && node->data == node->key);
    int key = *(const int *)node->key;
    REQUIRE(key > lower && key < upper);
    tree_info_t left = validate_subtree(node->left_child_node, node, lower, key);
    tree_info_t right = validate_subtree(node->right_child_node, node, key, upper);
    REQUIRE(node->left_child_count == left.size);
    REQUIRE(node->right_child_count == right.size);
    int diff = left.height - right.height;
    REQUIRE(diff >= -1 && diff <= 1);
    int height = 1 + (left.height > right.height ? left.height : right.height);
    REQUIRE(node->depth == (unsigned int)height);
    tree_info_t current = {1u + left.size + right.size, height};
    return current;
}

static void check_state(bostree_t *tree, const unsigned char present[KEYS],
                        bostree_node_t *nodes[KEYS],
                        bostree_node_t *held[KEYS]) {
    size_t count = 0;
    for (unsigned int k = 0; k < KEYS; ++k) {
        if (present[k]) {
            REQUIRE(nodes[k] != NULL && bostree_node_is_valid(nodes[k]));
            REQUIRE(bostree_lookup(tree, &key_values[k]) == nodes[k]);
            ++count;
        } else {
            REQUIRE(nodes[k] == NULL);
            REQUIRE(bostree_lookup(tree, &key_values[k]) == NULL);
        }
        if (held[k] != NULL && !bostree_node_is_valid(held[k])) {
            REQUIRE(bostree_node_key(held[k]) == NULL);
            REQUIRE(bostree_node_data(held[k]) == NULL);
        }
    }
    REQUIRE(bostree_node_count(tree) == count);
    REQUIRE(validate_subtree(tree->root_node, NULL, -1, (int)KEYS).size == count);
    size_t rank = 0;
    bostree_node_t *previous = NULL;
    for (unsigned int k = 0; k < KEYS; ++k) {
        if (!present[k]) continue;
        bostree_node_t *node = bostree_select(tree, (unsigned int)rank);
        REQUIRE(node == nodes[k]);
        REQUIRE(bostree_rank(node) == rank);
        REQUIRE(bostree_prev_node(node) == previous);
        if (previous != NULL) REQUIRE(bostree_next_node(previous) == node);
        previous = node;
        ++rank;
    }
    REQUIRE(bostree_select(tree, (unsigned int)count) == NULL);
    if (previous != NULL) REQUIRE(bostree_next_node(previous) == NULL);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096u) size = 4096u;
    memset(finalized, 0, sizeof(finalized));
    memset(inserted, 0, sizeof(inserted));
    for (unsigned int k = 0; k < KEYS; ++k) key_values[k] = (int)k;
    unsigned char present[KEYS] = {0};
    bostree_node_t *nodes[KEYS] = {0};
    bostree_node_t *held[KEYS] = {0};
    bostree_t *tree = bostree_new(compare_int, finalize_node);
    REQUIRE(tree != NULL);
    for (size_t p = 0; p + 1 < size; p += 2u) {
        unsigned int k = (unsigned int)data[p] % KEYS;
        unsigned int op = (unsigned int)data[p + 1u] % 7u;
        if (op <= 2u) {
            bostree_node_t *node = bostree_insert(tree, &key_values[k], &key_values[k]);
            REQUIRE(node != NULL);
            if (present[k]) REQUIRE(node == nodes[k]);
            else { present[k] = 1; nodes[k] = node; ++inserted[k]; }
        } else if (op == 3u) {
            if (present[k]) {
                bostree_remove(tree, nodes[k]);
                nodes[k] = NULL;
                present[k] = 0;
            }
        } else if (op == 4u) {
            if (present[k] && held[k] == NULL)
                held[k] = bostree_node_weak_ref(nodes[k]);
        } else if (op == 5u) {
            if (held[k] != NULL) {
                (void)bostree_node_weak_unref(NULL, held[k]);
                held[k] = NULL;
            }
        } else {
            REQUIRE(bostree_lookup(tree, &key_values[k]) ==
                    (present[k] ? nodes[k] : NULL));
        }
        check_state(tree, present, nodes, held);
    }
    bostree_destroy(tree);
    for (unsigned int k = 0; k < KEYS; ++k) {
        REQUIRE(finalized[k] == inserted[k]);
        if (held[k] != NULL) {
            REQUIRE(!bostree_node_is_valid(held[k]));
            REQUIRE(bostree_node_weak_unref(NULL, held[k]) == NULL);
        }
    }
    return 0;
}

#ifndef LEADERBOARD_LIBFUZZER
static uint32_t next_random(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *state = x;
    return x;
}
int main(void) {
    uint8_t input[2048];
    for (uint32_t seed = 1; seed <= 12u; ++seed) {
        uint32_t state = seed * UINT32_C(0x9e3779b9);
        for (size_t i = 0; i < sizeof(input); ++i)
            input[i] = (uint8_t)next_random(&state);
        REQUIRE(LLVMFuzzerTestOneInput(input, sizeof(input)) == 0);
    }
    for (size_t i = 0; i < sizeof(input); i += 2) {
        input[i] = (uint8_t)((i / 2) % KEYS);
        input[i + 1] = (uint8_t)((i / (2u * KEYS)) % 4u == 0 ? 0 : 3);
    }
    REQUIRE(LLVMFuzzerTestOneInput(input, sizeof(input)) == 0);
    return EXIT_SUCCESS;
}
#endif
