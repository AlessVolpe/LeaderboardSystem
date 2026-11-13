/*
 * LeaderboardSystem BOSTree (milestones 3.0.3-3.0.4).
 *
 * Copyright (c) 2017, Phillip Berndt
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Based on https://github.com/phillipberndt/bostree
 * Original implementation: GNU General Public License, version 3
 * or (at your option) any later version.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * See THIRD_PARTY_NOTICES.md for upstream attribution and modification history.
 */

#include "bostree_internal.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stable public accessors; callbacks may inspect payloads during disposal. */
const void *bostree_node_key(const bostree_node_t *node) {
    return node == NULL ? NULL : node->key;
}

void *bostree_node_data(const bostree_node_t *node) {
    return node == NULL ? NULL : node->data;
}

bool bostree_node_is_valid(const bostree_node_t *node) {
    return node != NULL && node->weak_ref_node_valid != 0U;
}

/* A leaf has depth 0; the height of an empty subtree is -1. */
static int _bostree_height(const BOSNode *node) {
    return node ? (int) node->depth : -1;
}

static unsigned int _bostree_size(const BOSNode *node) {
    return node ? node->left_child_count + node->right_child_count + 1U : 0U;
}

static int _imax(const int a, const int b) {
    return a > b ? a : b;
}

/* Recompute rather than increment metadata speculatively during descent. */
static void _bostree_update(BOSNode *node) {
    node->left_child_count = _bostree_size(node->left_child_node);
    node->right_child_count = _bostree_size(node->right_child_node);
    node->depth = (unsigned int) (1 + _imax(_bostree_height(node->left_child_node),
                                            _bostree_height(node->right_child_node)));
}

/* Positive balance means right-heavy. */
static int _bostree_balance(const BOSNode *node) {
    return _bostree_height(node->right_child_node) -
           _bostree_height(node->left_child_node);
}

static void _bostree_replace_child(BOSTree *tree, BOSNode *old_node,
                                   BOSNode *replacement) {
    BOSNode *grandparent = old_node->parent_node;
    if (replacement != NULL) replacement->parent_node = grandparent;

    if (grandparent == NULL) {
        tree->root_node = replacement;
    } else if (grandparent->left_child_node == old_node) {
        grandparent->left_child_node = replacement;
    } else {
        assert(grandparent->right_child_node == old_node);
        grandparent->right_child_node = replacement;
    }
}

static BOSNode *_bostree_rotate_right(BOSTree *tree, BOSNode *parent) {
    BOSNode *left = parent->left_child_node;
    assert(left != NULL);

    parent->left_child_node = left->right_child_node;
    if (parent->left_child_node != NULL)
        parent->left_child_node->parent_node = parent;

    _bostree_replace_child(tree, parent, left);
    left->right_child_node = parent;
    parent->parent_node = left;

    _bostree_update(parent);
    _bostree_update(left);
    return left;
}

static BOSNode *_bostree_rotate_left(BOSTree *tree, BOSNode *parent) {
    BOSNode *right = parent->right_child_node;
    assert(right != NULL);

    parent->right_child_node = right->left_child_node;
    if (parent->right_child_node != NULL)
        parent->right_child_node->parent_node = parent;

    _bostree_replace_child(tree, parent, right);
    right->left_child_node = parent;
    parent->parent_node = right;

    _bostree_update(parent);
    _bostree_update(right);
    return right;
}

/* Recalculate heights and order-statistics counts to the root. */
static void _bostree_rebalance_up(BOSTree *tree, BOSNode *node) {
    while (node != NULL) {
        _bostree_update(node);
        const int balance = _bostree_balance(node);

        if (balance < -1) {
            if (_bostree_balance(node->left_child_node) > 0)
                _bostree_rotate_left(tree, node->left_child_node);
            node = _bostree_rotate_right(tree, node);
        } else if (balance > 1) {
            if (_bostree_balance(node->right_child_node) < 0)
                _bostree_rotate_right(tree, node->right_child_node);
            node = _bostree_rotate_left(tree, node);
        }
        node = node->parent_node;
    }
}

static BOSNode *_bostree_new_node(BOSNode *parent_node, void *key, void *data) {
    BOSNode *node = calloc(1, sizeof(*node));
    if (node == NULL) return NULL;
    node->key = key;
    node->data = data;
    node->weak_ref_count = 1;
    node->weak_ref_node_valid = 1;
    node->parent_node = parent_node;
    return node;
}

BOSTree *bostree_new(const BOSTree_cmp_func cmp_func, const BOSTree_free_func free_func) {
    if (cmp_func == NULL) return NULL;

    BOSTree *tree = malloc(sizeof(*tree));
    if (tree == NULL) return NULL;
    tree->root_node = NULL;
    tree->cmp_func = cmp_func;
    tree->free_func = free_func;
    return tree;
}

/*
 * Invalidate a detached node. The tree's ownership reference is released here,
 * but external references keep the tombstone allocated until their last unref.
 * Payload finalization occurs exactly once, while the tree is still alive.
 */
static void _bostree_dispose(BOSTree *tree, BOSNode *node) {
    assert(tree != NULL && node != NULL);
    assert(node->weak_ref_node_valid && node->weak_ref_count > 0);

    node->weak_ref_node_valid = 0;
    node->left_child_node = NULL;
    node->right_child_node = NULL;
    node->parent_node = NULL;
    node->left_child_count = 0;
    node->right_child_count = 0;
    node->depth = 0;

    if (tree->free_func != NULL) tree->free_func(node);
    node->key = NULL;
    node->data = NULL;

    if (--node->weak_ref_count == 0) free(node);
}

void bostree_destroy(BOSTree *tree) {
    if (tree == NULL) return;

    /* Iterative postorder disposal; retains O(n) time and O(1) extra memory. */
    BOSNode *node = tree->root_node;
    while (node != NULL) {
        while (node->left_child_node != NULL) node = node->left_child_node;
        if (node->right_child_node != NULL) {
            node = node->right_child_node;
            continue;
        }

        BOSNode *parent = node->parent_node;
        if (parent != NULL) {
            if (parent->left_child_node == node) parent->left_child_node = NULL;
            else parent->right_child_node = NULL;
        }
        _bostree_dispose(tree, node);
        node = parent;
    }
    free(tree);
}

unsigned int bostree_node_count(const BOSTree *tree) {
    return tree ? _bostree_size(tree->root_node) : 0U;
}

BOSNode *bostree_insert(BOSTree *tree, void *key, void *data) {
    if (tree == NULL || key == NULL) return NULL;

    BOSNode **link = &tree->root_node;
    BOSNode *parent = NULL;

    while (*link != NULL) {
        parent = *link;
        const int cmp = tree->cmp_func(key, parent->key);
        if (cmp == 0) return parent; /* Duplicate: leave both key and data unchanged. */
        link = cmp < 0 ? &parent->left_child_node : &parent->right_child_node;
    }

    if (bostree_node_count(tree) == UINT_MAX) return NULL;

    BOSNode *inserted = _bostree_new_node(parent, key, data);
    if (inserted == NULL) return NULL;
    *link = inserted;
    _bostree_rebalance_up(tree, inserted);
    return inserted;
}

void bostree_remove(BOSTree *tree, BOSNode *node) {
    if (tree == NULL || node == NULL || !node->weak_ref_node_valid) return;

    /* An attached node must reach this tree's root via parent links. */
    BOSNode *root = node;
    while (root->parent_node != NULL) root = root->parent_node;
    if (root != tree->root_node) return;

    BOSNode *rebalance_from;
    if (node->left_child_node == NULL || node->right_child_node == NULL) {
        BOSNode *child = node->left_child_node != NULL
                             ? node->left_child_node : node->right_child_node;
        BOSNode *parent = node->parent_node;
        _bostree_replace_child(tree, node, child);
        rebalance_from = parent != NULL ? parent : child;
    } else {
        /* Physically transplant the successor. Swapping keys or payloads would
         * invalidate references to both nodes and alter ownership semantics. */
        BOSNode *successor = node->right_child_node;
        while (successor->left_child_node != NULL)
            successor = successor->left_child_node;

        if (successor->parent_node == node) {
            _bostree_replace_child(tree, node, successor);
            successor->left_child_node = node->left_child_node;
            successor->left_child_node->parent_node = successor;
            rebalance_from = successor;
        } else {
            BOSNode *old_parent = successor->parent_node;
            _bostree_replace_child(tree, successor, successor->right_child_node);
            _bostree_replace_child(tree, node, successor);
            successor->left_child_node = node->left_child_node;
            successor->right_child_node = node->right_child_node;
            successor->left_child_node->parent_node = successor;
            successor->right_child_node->parent_node = successor;
            /* The old successor parent now reaches the transplanted root. */
            rebalance_from = old_parent;
        }
    }

    _bostree_rebalance_up(tree, rebalance_from);
    _bostree_dispose(tree, node);
}

BOSNode *bostree_node_weak_ref(BOSNode *node) {
    if (node == NULL || !node->weak_ref_node_valid ||
        node->weak_ref_count == 0U || node->weak_ref_count == UCHAR_MAX) return NULL;
    ++node->weak_ref_count;
    return node;
}

BOSNode *bostree_node_weak_unref(const BOSTree *tree, BOSNode *node) {
    /* Retained invalid nodes must be releasable after bostree_destroy(tree). */
    (void) tree;
    if (node == NULL) return NULL;

    /* The tree owns the final reference while a node is still attached. */
    if (node->weak_ref_node_valid && node->weak_ref_count == 1U)
        return node;

    assert(node->weak_ref_count != 0U);
    if (--node->weak_ref_count == 0U) {
        free(node);
        return NULL;
    }
    return node->weak_ref_node_valid ? node : NULL;
}

BOSNode *bostree_lookup(const BOSTree *tree, const void *key) {
    if (tree == NULL || key == NULL) return NULL;

    BOSNode *node = tree->root_node;
    while (node != NULL) {
        const int cmp = tree->cmp_func(key, node->key);
        if (cmp == 0) return node;
        node = cmp < 0 ? node->left_child_node : node->right_child_node;
    }
    return NULL;
}

BOSNode *bostree_select(const BOSTree *tree, unsigned int index) {
    if (tree == NULL || index >= bostree_node_count(tree)) return NULL;

    BOSNode *node = tree->root_node;
    while (node != NULL) {
        if (index < node->left_child_count) {
            node = node->left_child_node;
        } else if (index == node->left_child_count) {
            return node;
        } else {
            index -= node->left_child_count + 1U;
            node = node->right_child_node;
        }
    }
    return NULL;
}

BOSNode *bostree_next_node(BOSNode *node) {
    if (node == NULL || !node->weak_ref_node_valid) return NULL;
    if (node->right_child_node) {
        node = node->right_child_node;
        while (node->left_child_node) node = node->left_child_node;
        return node;
    }

    while (node->parent_node && node->parent_node->right_child_node == node)
        node = node->parent_node;
    return node->parent_node;
}

BOSNode *bostree_prev_node(BOSNode *node) {
    if (node == NULL || !node->weak_ref_node_valid) return NULL;
    if (node->left_child_node) {
        node = node->left_child_node;
        while (node->right_child_node) node = node->right_child_node;
        return node;
    }

    while (node->parent_node && node->parent_node->left_child_node == node)
        node = node->parent_node;
    return node->parent_node;
}

unsigned int bostree_rank(const BOSNode *node) {
    if (node == NULL || !node->weak_ref_node_valid) return UINT_MAX;
    unsigned int rank = node->left_child_count;
    while (node->parent_node != NULL) {
        if (node->parent_node->right_child_node == node)
            rank += node->parent_node->left_child_count + 1U;
        node = node->parent_node;
    }
    return rank;
}

#if !defined(NDEBUG)

/* Debug helpers: print trees containing NUL-terminated string keys as DOT. */
static void _bostree_print_helper(const BOSNode *node) {
    printf("  %s [label=\"\\N (%u,%u,%u)\"];\n", (char *) node->key,
           node->left_child_count, node->right_child_count, node->depth);
    if (node->parent_node) printf("  %s -> %s [color=green];\n", (char *) node->key, (char *) node->parent_node->key);
    if (node->left_child_node != NULL) {
        printf("  %s -> %s\n", (char *) node->key, (char *) node->left_child_node->key);
        _bostree_print_helper(node->left_child_node);
    }
    if (node->right_child_node != NULL) {
        printf("  %s -> %s\n", (char *) node->key, (char *) node->right_child_node->key);
        _bostree_print_helper(node->right_child_node);
    }
}

void bostree_print(const BOSTree *tree) {
    if (tree == NULL || tree->root_node == NULL) return;
    printf("digraph {\n  ordering = out;\n");
    _bostree_print_helper(tree->root_node);
    printf("}\n");
    fflush(stdout);
}

#endif
