/* Milestone 3.0.3: deterministic structural and order-statistics regression tests. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bostree.h"

#define CHECK(condition) do {                                                       \
    if (!(condition)) {                                                            \
        fprintf(stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #condition);    \
        exit(EXIT_FAILURE);                                                        \
    }                                                                              \
} while (0)

typedef struct {
    unsigned int size;
    int height;
} subtree_info;

static unsigned int freed_nodes;

static void node_finalizer(BOSNode* node) {
    CHECK(node != NULL);
    ++freed_nodes;
}

static int compare_ints(const void* a, const void* b) {
    const int lhs = *(const int*)a;
    const int rhs = *(const int*)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int maximum(int a, int b) { return a > b ? a : b; }

/* Validate every AVL and order-statistics invariant, independent of the library. */
static subtree_info check_subtree(const BOSNode* node, const BOSNode* parent,
                                  int minimum, int maximum_key, unsigned int remaining_depth) {
    if (node == NULL) return (subtree_info){ 0U, -1 };

    CHECK(remaining_depth != 0U); /* Detect runaway structural cycles. */
    CHECK(node->parent_node == parent);
    CHECK(node->key != NULL);
    const int key = *(const int*)node->key;
    CHECK(key > minimum && key < maximum_key);
    CHECK(node->weak_ref_count == 1);
    CHECK(node->weak_ref_node_valid == 1);

    const subtree_info left = check_subtree(node->left_child_node, node,
                                            minimum, key, remaining_depth - 1);
    const subtree_info right = check_subtree(node->right_child_node, node,
                                             key, maximum_key, remaining_depth - 1);
    CHECK(node->left_child_count == left.size);
    CHECK(node->right_child_count == right.size);
    CHECK(node->depth == (unsigned int)(1 + maximum(left.height, right.height)));
    CHECK(left.height - right.height <= 1);
    CHECK(right.height - left.height <= 1);

    return (subtree_info){ left.size + right.size + 1U, 1 + maximum(left.height, right.height) };
}

static void check_tree(const BOSTree* tree, unsigned int size) {
    CHECK(tree != NULL);
    CHECK(bostree_node_count(tree) == size);
    CHECK((tree->root_node == NULL) == (size == 0U));
    if (size != 0U) CHECK(tree->root_node->parent_node == NULL);
    CHECK(check_subtree(tree->root_node, NULL, INT_MIN, INT_MAX, size + 1U).size == size);
    CHECK(bostree_select(tree, size) == NULL);
}

static void check_rank_and_navigation(const BOSTree* tree, int first, int step, unsigned int size) {
    BOSNode* previous = NULL;
    for (unsigned int i = 0; i < size; ++i) {
        BOSNode* node = bostree_select(tree, i);
        CHECK(node != NULL);
        CHECK(*(const int*)node->key == first + (int)i * step);
        CHECK(bostree_rank(node) == i);
        CHECK(bostree_lookup(tree, node->key) == node);
        CHECK(bostree_prev_node(node) == previous);
        if (previous != NULL) CHECK(bostree_next_node(previous) == node);
        previous = node;
    }
    if (previous != NULL) CHECK(bostree_next_node(previous) == NULL);
    CHECK(bostree_select(tree, size) == NULL);
}

static void test_empty(void) {
    CHECK(bostree_new(NULL, NULL) == NULL);
    CHECK(bostree_node_count(NULL) == 0U);
    CHECK(bostree_lookup(NULL, NULL) == NULL);
    CHECK(bostree_select(NULL, 0) == NULL);
    CHECK(bostree_next_node(NULL) == NULL);
    CHECK(bostree_prev_node(NULL) == NULL);
    bostree_destroy(NULL);

    BOSTree* tree = bostree_new(compare_ints, node_finalizer);
    CHECK(tree != NULL);
    check_tree(tree, 0);
    int value = 1;
    CHECK(bostree_lookup(tree, &value) == NULL);
    CHECK(bostree_insert(tree, NULL, &value) == NULL);
    CHECK(bostree_insert(NULL, &value, NULL) == NULL);
    CHECK(bostree_node_count(tree) == 0U);
    bostree_destroy(tree);
}

static void test_rotation(const int order[3], int expected_root) {
    int keys[3] = { order[0], order[1], order[2] };
    int values[3] = { 10, 20, 30 };
    freed_nodes = 0;
    BOSTree* tree = bostree_new(compare_ints, node_finalizer);
    CHECK(tree != NULL);

    for (unsigned int i = 0; i < 3; ++i) {
        CHECK(bostree_insert(tree, &keys[i], &values[i]) != NULL);
        check_tree(tree, i + 1U);
    }
    CHECK(*(const int*)tree->root_node->key == expected_root);
    check_rank_and_navigation(tree, 10, 10, 3);

    int duplicate = 20;
    int different_value = -1;
    BOSNode* original = bostree_lookup(tree, &duplicate);
    CHECK(original != NULL);
    void* original_data = original->data;
    CHECK(bostree_insert(tree, &duplicate, &different_value) == original);
    CHECK(original->data == original_data);
    CHECK(original->key != &duplicate);
    check_tree(tree, 3);

    bostree_destroy(tree);
    CHECK(freed_nodes == 3U);
}

static uint32_t next_random(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}

static void test_sequence(unsigned int size, int mode) {
    int* keys = malloc(size * sizeof(*keys));
    int* values = malloc(size * sizeof(*values));
    unsigned int* order = malloc(size * sizeof(*order));
    CHECK(keys != NULL && values != NULL && order != NULL);

    for (unsigned int i = 0; i < size; ++i) {
        keys[i] = (int)i;
        values[i] = (int)(i * 3U);
        order[i] = mode == 1 ? size - 1U - i : i;
    }
    if (mode == 2) {
        uint32_t random_state = UINT32_C(0xC0FFEE21);
        for (unsigned int i = size - 1U; i > 0U; --i) {
            const unsigned int j = next_random(&random_state) % (i + 1U);
            const unsigned int tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }
    }

    freed_nodes = 0;
    BOSTree* tree = bostree_new(compare_ints, node_finalizer);
    CHECK(tree != NULL);
    for (unsigned int i = 0; i < size; ++i) {
        unsigned int index = order[i];
        BOSNode* node = bostree_insert(tree, &keys[index], &values[index]);
        CHECK(node != NULL);
        CHECK(node->data == &values[index]);
        if ((i & 31U) == 0U || i + 1U == size)
            check_tree(tree, i + 1U);
    }

    check_rank_and_navigation(tree, 0, 1, size);
    for (unsigned int i = 0; i < size; ++i) {
        CHECK(bostree_lookup(tree, &keys[i]) != NULL);
        CHECK(bostree_lookup(tree, &keys[i])->data == &values[i]);
    }

    /* A live weak reference can be created and released without freeing a node. */
    BOSNode* mid = bostree_select(tree, size / 2U);
    CHECK(bostree_node_weak_ref(mid) == mid);
    CHECK(mid->weak_ref_count == 2U);
    CHECK(bostree_node_weak_unref(tree, mid) == mid);
    CHECK(mid->weak_ref_count == 1U);

    bostree_destroy(tree);
    CHECK(freed_nodes == size);
    free(order);
    free(values);
    free(keys);
}

int main(void) {
    const int ll[] = { 30, 20, 10 };
    const int rr[] = { 10, 20, 30 };
    const int lr[] = { 30, 10, 20 };
    const int rl[] = { 10, 30, 20 };

    test_empty();
    test_rotation(ll, 20);
    test_rotation(rr, 20);
    test_rotation(lr, 20);
    test_rotation(rl, 20);
    test_sequence(2048U, 0); /* Ascending, frequent right rotations. */
    test_sequence(2048U, 1); /* Descending, frequent left rotations. */
    test_sequence(4096U, 2); /* Fixed-seed random order. */
    return EXIT_SUCCESS;
}
