/*
 * Self-Balancing order statistic tree
 *
 * Copyright (c) 2017, Phillip Berndt
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
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

//
// Created by aless on 09/03/2024.
//

#ifndef BOSTREE_H
#define BOSTREE_H

#include <stdio.h>

typedef struct _BOSNode {
    unsigned int left_child_count;
    unsigned int right_child_count;
    unsigned int depth;

    struct _BOSNode *left_child_node;
    struct _BOSNode *right_child_node;
    struct _BOSNode *parent_node;

    void *key;
    void *data;

    unsigned char weak_ref_count;
    unsigned char weak_ref_node_valid;
} BOSNode;

typedef int (*BOSTree_cmp_func)(const void *, const void *);

typedef void (*BOSTree_free_func)(BOSNode *node);

typedef struct _BOSTree {
    BOSNode *root_node;

    BOSTree_cmp_func cmp_func;
    BOSTree_free_func free_func;
} BOSTree;


/* Comparator equality identifies a unique key. Inserting a duplicate returns the
 * existing node without changing its key/data. Null return means invalid input,
 * allocation failure, or a full (UINT_MAX-sized) tree. Keys and values are
 * borrowed unless free_func explicitly releases them at node destruction. */
BOSTree *bostree_new(BOSTree_cmp_func cmp_func, BOSTree_free_func free_func);

void bostree_destroy(BOSTree *tree);

unsigned int bostree_node_count(const BOSTree *tree);

BOSNode *bostree_insert(BOSTree *tree, void *key, void *data);

/* Remove an attached node, rebalance in O(log n), and dispose its payload.
 * Does nothing for NULL, invalidated nodes, or nodes in another live tree.
 * A valid pointer to an attached node is required; dangling pointers are UB.
 * Nodes retained by weak_ref become invalid tombstones on removal. */
void bostree_remove(BOSTree *tree, BOSNode *node);

/* Retain a live node. Returns NULL if invalid or the 8-bit count is full.
 * The tree holds one reference; callers release only references they acquired.
 * The finalizer is invoked on removal/destroy (not the last external unref).
 * After invalidation the retained node has weak_ref_node_valid == 0, NULL
 * key/data/links, and can only be inspected for validity and released.
 * A retained node can be released after tree destruction by passing NULL for
 * tree. For compatibility the tree argument is otherwise ignored.
 * Both functions require a non-dangling node or NULL. Never release twice. */
BOSNode *bostree_node_weak_ref(BOSNode *node);

BOSNode *bostree_node_weak_unref(const BOSTree *tree, BOSNode *node);

BOSNode *bostree_lookup(const BOSTree *tree, const void *key);

BOSNode *bostree_select(const BOSTree *tree, unsigned int index);

BOSNode *bostree_next_node(BOSNode *node);

BOSNode *bostree_prev_node(BOSNode *node);

/* Zero-based rank for attached nodes; UINT_MAX for NULL/invalidated nodes. */
unsigned int bostree_rank(const BOSNode *node);

#if !defined(NDEBUG)
/* DOT debug output requires NUL-terminated string keys. */
void bostree_print(const BOSTree *tree);

#define bostree_debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define bostree_debug(...) ((void)0)
#endif

#endif //BOSTREE_H
