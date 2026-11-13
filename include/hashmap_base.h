/*
 * LeaderboardSystem hashmap (milestones 3.0.2, 3.0.6).
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: MIT AND GPL-3.0-only
 *
 * Based on Hashmap by David Leeds: https://github.com/DavidLeeds/hashmap
 * Original portions are licensed under the MIT License.
 * The full upstream MIT notice and license are preserved in
 * THIRD_PARTY_NOTICES.md. The combined project distributes under GPLv3.
 */

#ifndef HASHMAP_BASE_H
#define HASHMAP_BASE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public hashmaps and iterator entries have opaque representations. */
typedef struct hashmap_base hashmap_base_t;
typedef struct hashmap_entry hashmap_entry_t;

/* Legacy spellings are retained as pointer-type aliases. The old stack-allocated
 * hashmap_base and direct-field interface are intentionally no longer public. */
typedef hashmap_base_t hashmap_base;
typedef hashmap_entry_t hashmap_entry;

/* Allocate/destroy a hashmap. NULL callbacks or allocation failure return
 * NULL. Caller owns the handle; borrowed keys and data are not released on destruction unless
 * key ownership callbacks have been explicitly registered. */
hashmap_base_t *hashmap_base_new(size_t (*hash_func)(const void *),
                                int (*compare_func)(const void *, const void *));
void hashmap_base_destroy(hashmap_base_t *hb);
size_t hashmap_base_size(const hashmap_base_t *hb);
/* Inspection only. Capacity is the allocated number of table slots. */
size_t hashmap_base_capacity(const hashmap_base_t *hb);
size_t hashmap_base_initial_capacity(const hashmap_base_t *hb);

/* Borrowed keys by default. To own copies, install BOTH callbacks on an empty map.
 * Returns -EINVAL for an incomplete pair, -EBUSY for a non-empty map. */
int hashmap_base_set_key_alloc_funcs(hashmap_base *hb,
                                     void *(*key_dup_func)(const void *),
                                     void (*key_free_func)(void *));

int hashmap_base_reserve(hashmap_base *hb, size_t capacity);

int hashmap_base_put(hashmap_base *hb, const void *key, const void *data);

void *hashmap_base_get(const hashmap_base *hb, const void *key);

void *hashmap_base_remove(hashmap_base *hb, const void *key);

/* Deprecated misspelling: retained as a source-level alias for recompiled clients.
 * No binary compatibility is promised for the 3.0.6 representation change. */
void *hahsmap_base_remove(hashmap_base *hb, const void *key);

void hashmap_base_clear(hashmap_base *hb);

void hashmap_base_reset(hashmap_base *hb);

hashmap_entry *hashmap_base_iter(const hashmap_base *hb, const hashmap_entry *pos);

bool hashmap_base_iter_valid(const hashmap_base *hb, const hashmap_entry *iter);

bool hashmap_base_iter_next(const hashmap_base *hb, hashmap_entry **iter);

bool hashmap_base_iter_remove(hashmap_base *hb, hashmap_entry **iter);

const void *hashmap_base_iter_get_key(const hashmap_entry *iter);

void *hashmap_base_iter_get_data(const hashmap_entry *iter);

int hashmap_base_iter_set_data(hashmap_entry *iter, void *data);

double hashmap_base_load_factor(const hashmap_base *hb);

size_t hashmap_base_collisions(const hashmap_base *hb, const void *key);

double hashmap_base_collisions_mean(const hashmap_base *hb);

double hashmap_base_collisions_variance(const hashmap_base *hb);

size_t hashmap_hash_default(const void *data, size_t len);

size_t hashmap_hash_string(const char *key);

size_t hashmap_hash_string_i(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* HASHMAP_BASE_H */
