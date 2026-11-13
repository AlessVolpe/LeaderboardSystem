/*
 * LeaderboardSystem hashmap private representation (milestone 3.0.6).
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: MIT AND GPL-3.0-only
 * Based on https://github.com/DavidLeeds/hashmap (MIT).
 * Internal only; see THIRD_PARTY_NOTICES.md.
 */
#ifndef LEADERBOARD_HASHMAP_INTERNAL_H
#define LEADERBOARD_HASHMAP_INTERNAL_H

#include "hashmap_base.h"

typedef struct hashmap_entry {
    void *key;
    void *data;
} hashmap_entry;

typedef struct hashmap_base {
    size_t table_size_init;
    size_t table_size;
    size_t size;
    hashmap_entry *table;
    size_t (*hash)(const void *);
    int (*compare)(const void *, const void *);
    void *(*key_dup)(const void *);
    void (*key_free)(void *);
} hashmap_base;

/* Stack lifecycle is internal to the implementation and white-box tests.
 * Public clients use heap-allocated hashmap_base_t handles. */
void hashmap_base_init(hashmap_base_t *hb, size_t (*hash_func)(const void *),
                       int (*compare_func)(const void *, const void *));
void hashmap_base_cleanup(hashmap_base_t *hb);

#endif /* LEADERBOARD_HASHMAP_INTERNAL_H */
