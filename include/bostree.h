/*
 * LeaderboardSystem BOSTree public API (milestone 3.0.6).
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

#ifndef BOSTREE_H
#define BOSTREE_H

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Tree and node representations are private to src/internal/. */
typedef struct bostree bostree_t;
typedef struct bostree_node bostree_node_t;

typedef int (*bostree_compare_fn_t)(const void *, const void *);
typedef void (*bostree_free_fn_t)(bostree_node_t *node);

/* Source-compatibility aliases for existing pointer-based clients. */
typedef bostree_t BOSTree;
typedef bostree_node_t BOSNode;
typedef bostree_compare_fn_t BOSTree_cmp_func;
typedef bostree_free_fn_t BOSTree_free_func;

/* Unique keys are determined by the comparator. Duplicate insertion returns
 * the existing node, without replacing its key/data. NULL on invalid input,
 * allocation failure, or a full (UINT_MAX-sized) tree. */
bostree_t *bostree_new(bostree_compare_fn_t cmp_func, bostree_free_fn_t free_func);
void bostree_destroy(bostree_t *tree);
unsigned int bostree_node_count(const bostree_t *tree);
bostree_node_t *bostree_insert(bostree_t *tree, void *key, void *data);

/* Requires a live attached node; removes it in O(log n). Retained node
 * handles are invalidated but remain allocated until last weak_unref.
 * NULL, invalidated and wrong-tree nodes are ignored. Dangling pointers UB. */
void bostree_remove(bostree_t *tree, bostree_node_t *node);

/* Despite historical names, weak_refs prolong the node allocation, not the
 * payload's lifetime. Never unref a reference you did not successfully obtain.
 * NULL is accepted. The tree argument to unref is ignored for compatibility;
 * use NULL after destruction. The counter is 8-bit, not thread-safe. */
bostree_node_t *bostree_node_weak_ref(bostree_node_t *node);
bostree_node_t *bostree_node_weak_unref(const bostree_t *tree, bostree_node_t *node);

/* Borrowed access to payloads. Both return NULL for a NULL node.
 * Keys must not be mutated while attached. Accessors do not transfer ownership.
 * An invalidated retained node has NULL
 * key/data after disposal. During a free_func callback, the node has already
 * been marked invalid, but key and data are still available through these
 * accessors until the callback returns. Never access a dangling node. */
const void *bostree_node_key(const bostree_node_t *node);
void *bostree_node_data(const bostree_node_t *node);
bool bostree_node_is_valid(const bostree_node_t *node);

bostree_node_t *bostree_lookup(const bostree_t *tree, const void *key);
bostree_node_t *bostree_select(const bostree_t *tree, unsigned int index);
bostree_node_t *bostree_next_node(bostree_node_t *node);
bostree_node_t *bostree_prev_node(bostree_node_t *node);
/* Zero-based rank; UINT_MAX for NULL or invalidated nodes. */
unsigned int bostree_rank(const bostree_node_t *node);

#if !defined(NDEBUG)
/* DOT debug output requires NUL-terminated string keys. */
void bostree_print(const bostree_t *tree);
#define bostree_debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define bostree_debug(...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* BOSTREE_H */
