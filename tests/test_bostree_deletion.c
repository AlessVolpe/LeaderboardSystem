/*
 * LeaderboardSystem BOSTree deletion tests (milestone 3.0.4).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bostree_internal.h"

#define CHECK(expr) do {                                                       \
    if (!(expr)) {                                                            \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr);            \
        exit(EXIT_FAILURE);                                                   \
    }                                                                         \
} while (0)

#define KEY_SPACE 256
static int keys[KEY_SPACE];
static int values[KEY_SPACE];
static unsigned int finalized[KEY_SPACE];
static unsigned int finalization_total;

static int cmp_int(const void *a, const void *b) {
    const int x = *(const int *)a;
    const int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void finalize_node(BOSNode *node) {
    CHECK(node != NULL && node->key != NULL);
    CHECK(node->weak_ref_node_valid == 0U);
    const int key = *(const int *)node->key;
    CHECK(key >= 0 && key < KEY_SPACE);
    CHECK(node->key == &keys[key]);
    CHECK(node->data == &values[key]);
    ++finalized[key];
    ++finalization_total;
}

typedef struct {
    unsigned int count;
    int height;
} info;

static int maximum(int a, int b) { return a > b ? a : b; }

static info validate_node(const BOSNode *node, const BOSNode *parent,
                          int minimum, int maximum_key, int budget,
                          const unsigned char present[KEY_SPACE],
                          BOSNode *const nodes[KEY_SPACE]) {
    if (node == NULL) return (info){0U, -1};
    CHECK(budget > 0);
    CHECK(node->parent_node == parent);
    CHECK(node->weak_ref_node_valid == 1U);
    CHECK(node->weak_ref_count >= 1U);
    CHECK(node->key != NULL);
    const int k = *(const int *)node->key;
    CHECK(k > minimum && k < maximum_key);
    CHECK(k >= 0 && k < KEY_SPACE);
    CHECK(present[k]);
    CHECK(nodes[k] == node); /* No key/data swaps or node identity changes. */
    CHECK(node->key == &keys[k]);
    CHECK(node->data == &values[k]);

    info left = validate_node(node->left_child_node, node, minimum, k,
                              budget - 1, present, nodes);
    info right = validate_node(node->right_child_node, node, k, maximum_key,
                               budget - 1, present, nodes);
    CHECK(node->left_child_count == left.count);
    CHECK(node->right_child_count == right.count);
    CHECK(node->depth == (unsigned int)(maximum(left.height, right.height) + 1));
    CHECK(left.height - right.height <= 1);
    CHECK(right.height - left.height <= 1);
    return (info){left.count + right.count + 1U,
                  maximum(left.height, right.height) + 1};
}

static void validate_tree(BOSTree *tree, const unsigned char present[KEY_SPACE],
                          BOSNode *const nodes[KEY_SPACE]) {
    CHECK(tree != NULL);
    unsigned int expected = 0;
    for (int i = 0; i < KEY_SPACE; ++i) expected += present[i] != 0U;
    CHECK(bostree_node_count(tree) == expected);
    CHECK((tree->root_node == NULL) == (expected == 0U));
    if (tree->root_node != NULL) CHECK(tree->root_node->parent_node == NULL);
    CHECK(validate_node(tree->root_node, NULL, INT_MIN, INT_MAX, KEY_SPACE + 1,
                        present, nodes).count == expected);
    CHECK(bostree_select(tree, expected) == NULL);

    unsigned int index = 0;
    BOSNode *previous = NULL;
    for (int key = 0; key < KEY_SPACE; ++key) {
        CHECK((bostree_lookup(tree, &keys[key]) != NULL) == (present[key] != 0U));
        if (!present[key]) continue;
        BOSNode *node = bostree_select(tree, index);
        CHECK(node == nodes[key]);
        CHECK(bostree_lookup(tree, &keys[key]) == node);
        CHECK(bostree_rank(node) == index);
        CHECK(bostree_prev_node(node) == previous);
        if (previous != NULL) CHECK(bostree_next_node(previous) == node);
        previous = node;
        ++index;
    }
    CHECK(index == expected);
    if (previous != NULL) CHECK(bostree_next_node(previous) == NULL);
}

static void setup_model(unsigned char present[KEY_SPACE],
                        BOSNode *nodes[KEY_SPACE]) {
    memset(present, 0, KEY_SPACE);
    memset(nodes, 0, KEY_SPACE * sizeof(nodes[0]));
}

static void insert_key(BOSTree *tree, int key,
                       unsigned char present[KEY_SPACE],
                       BOSNode *nodes[KEY_SPACE]) {
    CHECK(!present[key]);
    nodes[key] = bostree_insert(tree, &keys[key], &values[key]);
    CHECK(nodes[key] != NULL);
    CHECK(nodes[key]->key == &keys[key]);
    present[key] = 1U;
}

static void remove_key(BOSTree *tree, int key,
                       unsigned char present[KEY_SPACE],
                       BOSNode *nodes[KEY_SPACE]) {
    CHECK(present[key]);
    unsigned int before = finalized[key];
    bostree_remove(tree, nodes[key]);
    CHECK(finalized[key] == before + 1U);
    present[key] = 0U;
    nodes[key] = NULL;
    CHECK(bostree_lookup(tree, &keys[key]) == NULL);
}

/* All 7! deletion orders exercise leaf, one-child, and deep successor cases. */
static void deletion_permutation(const int order[7]) {
    unsigned char present[KEY_SPACE];
    BOSNode *nodes[KEY_SPACE];
    setup_model(present, nodes);
    BOSTree *tree = bostree_new(cmp_int, finalize_node);
    CHECK(tree != NULL);
    const int insertion_order[7] = {3, 1, 5, 0, 2, 4, 6};
    for (int i = 0; i < 7; ++i) {
        insert_key(tree, insertion_order[i], present, nodes);
    }
    validate_tree(tree, present, nodes);
    for (int i = 0; i < 7; ++i) {
        remove_key(tree, order[i], present, nodes);
        validate_tree(tree, present, nodes);
    }
    bostree_destroy(tree);
}

static void permute(int order[7], int pos) {
    if (pos == 7) {
        deletion_permutation(order);
        return;
    }
    for (int i = pos; i < 7; ++i) {
        int temp = order[pos]; order[pos] = order[i]; order[i] = temp;
        permute(order, pos + 1);
        temp = order[pos]; order[pos] = order[i]; order[i] = temp;
    }
}

static uint32_t random_next(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}

static void randomized_reference_test(void) {
    unsigned char present[KEY_SPACE];
    BOSNode *nodes[KEY_SPACE];
    setup_model(present, nodes);
    BOSTree *tree = bostree_new(cmp_int, finalize_node);
    CHECK(tree != NULL);
    uint32_t rng = UINT32_C(0xA731A904);
    for (unsigned int step = 0; step < 15000U; ++step) {
        const int k = (int)(random_next(&rng) % KEY_SPACE);
        if (present[k]) {
            if ((random_next(&rng) & 3U) != 0U) {
                remove_key(tree, k, present, nodes);
            } else {
                CHECK(bostree_insert(tree, &keys[k], &values[k]) == nodes[k]);
            }
        } else {
            insert_key(tree, k, present, nodes);
        }
        if ((step % 17U) == 0U) validate_tree(tree, present, nodes);
    }
    validate_tree(tree, present, nodes);
    for (int k = 0; k < KEY_SPACE; ++k) {
        if (present[k]) remove_key(tree, k, present, nodes);
        if ((k % 13) == 0) validate_tree(tree, present, nodes);
    }
    validate_tree(tree, present, nodes);
    bostree_destroy(tree);
}

static void weak_reference_lifecycle_test(void) {
    BOSTree *tree = bostree_new(cmp_int, finalize_node);
    BOSTree *other = bostree_new(cmp_int, finalize_node);
    CHECK(tree != NULL && other != NULL);
    BOSNode *root = bostree_insert(tree, &keys[30], &values[30]);
    BOSNode *left = bostree_insert(tree, &keys[20], &values[20]);
    BOSNode *right = bostree_insert(tree, &keys[40], &values[40]);
    BOSNode *deep = bostree_insert(tree, &keys[35], &values[35]);
    CHECK(root != NULL && left != NULL && right != NULL && deep != NULL);

    bostree_remove(other, root); /* Wrong tree must not mutate anything. */
    CHECK(bostree_node_count(tree) == 4U);
    CHECK(bostree_node_count(other) == 0U);
    bostree_remove(NULL, root);
    bostree_remove(tree, NULL);
    CHECK(bostree_node_count(tree) == 4U);

    CHECK(bostree_node_weak_ref(root) == root);
    CHECK(bostree_node_weak_ref(root) == root);
    CHECK(bostree_node_weak_ref(deep) == deep);
    unsigned int was_finalized = finalized[30];
    bostree_remove(tree, root); /* Two children, deep successor, stable identity. */
    CHECK(finalized[30] == was_finalized + 1U);
    CHECK(bostree_lookup(tree, &keys[35]) == deep);
    CHECK(bostree_lookup(tree, &keys[40]) == right);
    CHECK(bostree_lookup(tree, &keys[20]) == left);
    CHECK(root->weak_ref_node_valid == 0U);
    CHECK(root->key == NULL && root->data == NULL);
    CHECK(root->parent_node == NULL && root->left_child_node == NULL &&
          root->right_child_node == NULL);
    CHECK(bostree_node_weak_ref(root) == NULL);
    CHECK(bostree_rank(root) == UINT_MAX);
    CHECK(bostree_next_node(root) == NULL);
    CHECK(bostree_prev_node(root) == NULL);
    bostree_remove(tree, root); /* Already invalid, still retained. */
    CHECK(finalized[30] == was_finalized + 1U);
    CHECK(bostree_node_weak_unref(NULL, root) == NULL);
    CHECK(root->weak_ref_count == 1U);
    CHECK(bostree_node_weak_unref(NULL, root) == NULL); /* Last external ref. */

    /* Destroying a tree invalidates outstanding handles immediately. */
    unsigned int left_before = finalized[20];
    unsigned int right_before = finalized[40];
    unsigned int deep_before = finalized[35];
    bostree_destroy(tree);
    CHECK(finalized[20] == left_before + 1U);
    CHECK(finalized[40] == right_before + 1U);
    CHECK(finalized[35] == deep_before + 1U);
    CHECK(deep->weak_ref_node_valid == 0U);
    CHECK(deep->key == NULL && deep->data == NULL);
    CHECK(bostree_node_weak_unref(NULL, deep) == NULL);
    bostree_destroy(other);

    CHECK(bostree_node_weak_ref(NULL) == NULL);
    CHECK(bostree_node_weak_unref(NULL, NULL) == NULL);
    CHECK(bostree_rank(NULL) == UINT_MAX);

    /* Saturation must not wrap the 8-bit reference count to zero. */
    tree = bostree_new(cmp_int, finalize_node);
    CHECK(tree != NULL);
    BOSNode *node = bostree_insert(tree, &keys[64], &values[64]);
    CHECK(node != NULL);
    for (unsigned int i = 1U; i < UCHAR_MAX; ++i)
        CHECK(bostree_node_weak_ref(node) == node);
    CHECK(node->weak_ref_count == UCHAR_MAX);
    CHECK(bostree_node_weak_ref(node) == NULL);
    for (unsigned int i = 1U; i < UCHAR_MAX; ++i)
        CHECK(bostree_node_weak_unref(tree, node) == node);
    CHECK(node->weak_ref_count == 1U);
    CHECK(bostree_node_weak_unref(tree, node) == node); /* Tree-owned ref. */
    bostree_destroy(tree);
}

int main(void) {
    for (int i = 0; i < KEY_SPACE; ++i) {
        keys[i] = i;
        values[i] = i * 9;
    }
    int permutation[7] = {0, 1, 2, 3, 4, 5, 6};
    permute(permutation, 0);
    randomized_reference_test();
    weak_reference_lifecycle_test();
    CHECK(finalization_total > 35000U); /* Tests actually exercised deletion. */
    return EXIT_SUCCESS;
}
