/*
 * LeaderboardSystem ranking abstraction (milestone 3.0.5).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ranking.h"

#include "bostree.h"
#include "hashmap_base.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Hash keys borrow these stable IDs. Tree keys borrow them as well. */
typedef struct ranking_entry {
    int64_t score;
    BOSNode *node;
    char id[];
} ranking_entry_t;

typedef struct ranking_order_key {
    int64_t score;
    const char *id;
} ranking_order_key_t;

typedef struct ranking {
    hashmap_base by_id;
    BOSTree *by_rank;
} ranking_t;

static size_t ranking_hash_id(const void *id) {
    return hashmap_hash_string((const char *)id);
}

static int ranking_compare_id(const void *lhs, const void *rhs) {
    return strcmp((const char *)lhs, (const char *)rhs);
}

/* Negative means higher priority (earlier in the tree). Avoid subtracting
 * signed scores: INT64_MIN/INT64_MAX must compare without overflow. */
static int ranking_compare_score(const void *lhs, const void *rhs) {
    const ranking_order_key_t *a = lhs;
    const ranking_order_key_t *b = rhs;
    if (a->score > b->score) return -1;
    if (a->score < b->score) return 1;
    return strcmp(a->id, b->id);
}

static void ranking_free_order_key(BOSNode *node) {
    free(node->key);
}

static ranking_order_key_t *ranking_make_key(const char *id, int64_t score) {
    ranking_order_key_t *key = malloc(sizeof(*key));
    if (key != NULL) {
        key->id = id;
        key->score = score;
    }
    return key;
}

ranking_t *ranking_new(void) {
    ranking_t *ranking = malloc(sizeof(*ranking));
    if (ranking == NULL) return NULL;
    hashmap_base_init(&ranking->by_id, ranking_hash_id, ranking_compare_id);
    ranking->by_rank = bostree_new(ranking_compare_score, ranking_free_order_key);
    if (ranking->by_rank == NULL) {
        hashmap_base_cleanup(&ranking->by_id);
        free(ranking);
        return NULL;
    }
    return ranking;
}

void ranking_destroy(ranking_t *ranking) {
    if (ranking == NULL) return;
    /* Finalizes tree-owned order keys before releasing their borrowed IDs. */
    bostree_destroy(ranking->by_rank);
    for (hashmap_entry *it = hashmap_base_iter(&ranking->by_id, NULL);
         it != NULL; (void)hashmap_base_iter_next(&ranking->by_id, &it)) {
        free(hashmap_base_iter_get_data(it));
    }
    hashmap_base_cleanup(&ranking->by_id);
    free(ranking);
}

size_t ranking_size(const ranking_t *ranking) {
    return ranking == NULL ? 0U : ranking->by_id.size;
}

int ranking_upsert(ranking_t *ranking, const char *id, int64_t score) {
    if (ranking == NULL || id == NULL || id[0] == '\0') return -EINVAL;

    ranking_entry_t *entry = hashmap_base_get(&ranking->by_id, id);
    if (entry != NULL) {
        if (entry->score == score) return 0;
        /* Temporarily insert a second tree node for the same ID. The new
         * score makes its composite ordering key distinct. The old node
         * remains untouched if either allocation fails. */
        if (bostree_node_count(ranking->by_rank) == UINT_MAX) return -EOVERFLOW;
        ranking_order_key_t *key = ranking_make_key(entry->id, score);
        if (key == NULL) return -ENOMEM;
        BOSNode *replacement = bostree_insert(ranking->by_rank, key, entry);
        if (replacement == NULL) {
            free(key);
            return -ENOMEM;
        }
        /* The BOSTree owns key after successful insertion. */
        bostree_remove(ranking->by_rank, entry->node);
        entry->node = replacement;
        entry->score = score;
        return 0;
    }

    if (bostree_node_count(ranking->by_rank) == UINT_MAX) return -EOVERFLOW;
    const size_t len = strlen(id);
    if (len > SIZE_MAX - sizeof(ranking_entry_t) - 1U) return -EOVERFLOW;
    entry = malloc(sizeof(*entry) + len + 1U);
    if (entry == NULL) return -ENOMEM;
    entry->score = score;
    memcpy(entry->id, id, len + 1U);

    ranking_order_key_t *key = ranking_make_key(entry->id, score);
    if (key == NULL) {
        free(entry);
        return -ENOMEM;
    }
    entry->node = bostree_insert(ranking->by_rank, key, entry);
    if (entry->node == NULL) {
        free(key);
        free(entry);
        return -ENOMEM;
    }

    const int result = hashmap_base_put(&ranking->by_id, entry->id, entry);
    if (result != 0) {
        /* The map has not accepted the borrowed identifier. Undo the tree
         * insertion first, then release the entry that owns the ID. */
        bostree_remove(ranking->by_rank, entry->node);
        free(entry);
        return result;
    }
    return 0;
}

int ranking_remove(ranking_t *ranking, const char *id) {
    if (ranking == NULL || id == NULL || id[0] == '\0') return -EINVAL;
    ranking_entry_t *entry = hashmap_base_remove(&ranking->by_id, id);
    if (entry == NULL) return -ENOENT;
    bostree_remove(ranking->by_rank, entry->node);
    free(entry);
    return 0;
}

int ranking_get(const ranking_t *ranking, const char *id,
                int64_t *score, size_t *rank) {
    if (ranking == NULL || id == NULL || id[0] == '\0') return -EINVAL;
    const ranking_entry_t *entry = hashmap_base_get(&ranking->by_id, id);
    if (entry == NULL) return -ENOENT;
    if (score != NULL) *score = entry->score;
    if (rank != NULL) *rank = (size_t)bostree_rank(entry->node);
    return 0;
}

int ranking_at(const ranking_t *ranking, size_t rank,
               const char **id, int64_t *score) {
    if (ranking == NULL) return -EINVAL;
    if (rank >= ranking_size(ranking)) return -ENOENT;
    const BOSNode *node = bostree_select(ranking->by_rank, (unsigned int)rank);
    if (node == NULL) return -ENOENT;
    const ranking_entry_t *entry = node->data;
    if (id != NULL) *id = entry->id;
    if (score != NULL) *score = entry->score;
    return 0;
}
