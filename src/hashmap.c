/*
 * LeaderboardSystem hashmap (milestone 3.0.2).
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: MIT AND GPL-3.0-only
 *
 * Based on Hashmap by David Leeds: https://github.com/DavidLeeds/hashmap
 * Original portions are licensed under the MIT License.
 * The full upstream MIT notice and license are preserved in
 * THIRD_PARTY_NOTICES.md. The combined project distributes under GPLv3.
 */

// Hashmap with open addressing, linear probing, and backward-shift deletion.
// This component owns its entry array and, optionally, duplicated keys.
// Values are always borrowed; see docs/hashmap-contract.md.

#include "hashmap_base.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_SIZE_MIN 32u
#define HASHMAP_SIZE_DEFAULT 128u

static size_t next_slot(const hashmap_base *hb, size_t slot) {
    return (slot + 1u) & (hb->table_size - 1u);
}

/* Round up to a power-of-two table large enough for a <= 0.75 load. */
static int table_size_for(size_t capacity, size_t minimum, size_t *result) {
    size_t count = minimum < HASHMAP_SIZE_MIN ? HASHMAP_SIZE_MIN : minimum;
    assert((count & (count - 1u)) == 0);

    while (capacity > count - count / 4u) {
        if (count > SIZE_MAX / 2u) return -EOVERFLOW;
        count *= 2u;
    }
    if (count > SIZE_MAX / sizeof(hashmap_entry)) return -EOVERFLOW;
    *result = count;
    return 0;
}

static size_t initial_slot(const hashmap_base *hb, const void *key) {
    size_t hash = hb->hash(key);
    /* Retain the existing secondary hash for compatibility. */
    return hashmap_hash_default(&hash, sizeof(hash)) & (hb->table_size - 1u);
}

static hashmap_entry *find_slot(const hashmap_base *hb, const void *key, bool empty_ok) {
    if (hb->table_size == 0) return NULL;
    size_t index = initial_slot(hb, key);
    for (size_t scanned = 0; scanned < hb->table_size; ++scanned) {
        hashmap_entry *entry = &hb->table[index];
        if (entry->key == NULL) return empty_ok ? entry : NULL;
        if (hb->compare(key, entry->key) == 0) return entry;
        index = next_slot(hb, index);
    }
    return NULL;
}

/* Atomic in case of allocation failure: keep the old table untouched. */
static int rehash(hashmap_base *hb, size_t count) {
    assert(count && (count & (count - 1u)) == 0);
    if (count > SIZE_MAX / sizeof(hashmap_entry)) return -EOVERFLOW;
    hashmap_entry *next = calloc(count, sizeof(*next));
    if (next == NULL) return -ENOMEM;

    hashmap_base updated = *hb;
    updated.table = next;
    updated.table_size = count;
    for (size_t i = 0; i < hb->table_size; ++i) {
        if (hb->table[i].key != NULL) {
            hashmap_entry *dest = find_slot(&updated, hb->table[i].key, true);
            assert(dest != NULL);
            *dest = hb->table[i]; /* Keys and data are never duplicated by rehash. */
        }
    }
    free(hb->table);
    hb->table = next;
    hb->table_size = count;
    return 0;
}

static void free_owned_keys(const hashmap_base *hb) {
    if (hb->key_free == NULL || hb->size == 0) return;
    for (size_t i = 0; i < hb->table_size; ++i) {
        if (hb->table[i].key != NULL) hb->key_free(hb->table[i].key);
    }
}

/* Preserve lookup correctness when deleting an entry from a probe cluster. */
static void erase_slot(hashmap_base *hb, hashmap_entry *entry) {
    size_t hole = (size_t) (entry - hb->table);
    if (hb->key_free != NULL) hb->key_free(entry->key);
    *entry = (hashmap_entry)
    {
        0
    };
    --hb->size;

    size_t index = next_slot(hb, hole);
    while (hb->table[index].key != NULL) {
        size_t home = initial_slot(hb, hb->table[index].key);
        size_t hole_distance = (hole - home) & (hb->table_size - 1u);
        size_t index_distance = (index - home) & (hb->table_size - 1u);
        if (hole_distance < index_distance) {
            hb->table[hole] = hb->table[index];
            hb->table[index] = (hashmap_entry)
            {
                0
            };
            hole = index;
        }
        index = next_slot(hb, index);
    }
}

void hashmap_base_init(hashmap_base * hb, size_t(*hash_func)(const void *),
                       int(*compare_func)(const void *, const void *)) {
    assert(hb && hash_func && compare_func);
    memset(hb, 0, sizeof(*hb));
    hb->table_size_init = HASHMAP_SIZE_DEFAULT;
    hb->hash = hash_func;
    hb->compare = compare_func;
}

void hashmap_base_cleanup(hashmap_base *hb) {
    if (hb == NULL) return;
    free_owned_keys(hb);
    free(hb->table);
    memset(hb, 0, sizeof(*hb));
}

int hashmap_base_set_key_alloc_funcs(hashmap_base * hb,
                                     void *(*key_dup_func)(const void *),
                                     void(*key_free_func)(void *)) {
    if (hb == NULL || ((key_dup_func == NULL) != (key_free_func == NULL))) return -EINVAL;
    if (hb->size != 0) return -EBUSY;
    hb->key_dup = key_dup_func;
    hb->key_free = key_free_func;
    return 0;
}

int hashmap_base_reserve(hashmap_base *hb, size_t capacity) {
    if (hb == NULL) return -EINVAL;
    size_t wanted;
    int rc = table_size_for(capacity, HASHMAP_SIZE_MIN, &wanted);
    if (rc != 0) return rc;
    if (wanted > hb->table_size) {
        rc = rehash(hb, wanted);
        if (rc != 0) return rc;
    }
    hb->table_size_init = wanted;
    return 0;
}

int hashmap_base_put(hashmap_base *hb, const void *key, const void *data) {
    if (hb == NULL || key == NULL || data == NULL) return -EINVAL;
    if (find_slot(hb, key, false) != NULL) return -EEXIST;
    if (hb->size == SIZE_MAX) return -EOVERFLOW;

    size_t wanted;
    int rc = table_size_for(hb->size + 1u, hb->table_size_init, &wanted);
    if (rc != 0) return rc;
    if (wanted > hb->table_size) {
        rc = rehash(hb, wanted);
        if (rc != 0) return rc;
    }

    hashmap_entry *slot = find_slot(hb, key, true);
    if (slot == NULL) return -ENOSPC;
    void *stored_key = hb->key_dup == NULL ? (void *) key : hb->key_dup(key);
    if (stored_key == NULL) return -ENOMEM;
    slot->key = stored_key;
    slot->data = (void *) data;
    ++hb->size;
    return 0;
}

void *hashmap_base_get(const hashmap_base *hb, const void *key) {
    if (hb == NULL || key == NULL) return NULL;
    hashmap_entry *entry = find_slot(hb, key, false);
    return entry == NULL ? NULL : entry->data;
}

void *hashmap_base_remove(hashmap_base *hb, const void *key) {
    if (hb == NULL || key == NULL) return NULL;
    hashmap_entry *entry = find_slot(hb, key, false);
    if (entry == NULL) return NULL;
    void *data = entry->data;
    erase_slot(hb, entry);
    return data;
}

/* ABI compatibility for code using the original misspelled symbol. */
void *hahsmap_base_remove(hashmap_base *hb, const void *key) {
    return hashmap_base_remove(hb, key);
}

void hashmap_base_clear(hashmap_base *hb) {
    if (hb == NULL) return;
    free_owned_keys(hb);
    hb->size = 0;
    if (hb->table != NULL) memset(hb->table, 0, hb->table_size * sizeof(*hb->table));
}

void hashmap_base_reset(hashmap_base *hb) {
    if (hb == NULL) return;
    hashmap_base_clear(hb);
    if (hb->table == NULL || hb->table_size == hb->table_size_init) return;
    /* A failed shrink is harmless: the empty table remains valid. */
    hashmap_entry *smaller = calloc(hb->table_size_init, sizeof(*smaller));
    if (smaller == NULL) return;
    free(hb->table);
    hb->table = smaller;
    hb->table_size = hb->table_size_init;
}

static hashmap_entry *next_populated(const hashmap_base *hb, const hashmap_entry *start) {
    if (hb == NULL || hb->size == 0 || hb->table == NULL) return NULL;
    for (const hashmap_entry *entry = start; entry < hb->table + hb->table_size; ++entry) {
        if (entry->key != NULL) return (hashmap_entry *) entry;
    }
    return NULL;
}

hashmap_entry *hashmap_base_iter(const hashmap_base *hb, const hashmap_entry *pos) {
    if (hb == NULL || hb->table == NULL) return NULL;
    return next_populated(hb, pos == NULL ? hb->table : pos);
}

bool hashmap_base_iter_valid(const hashmap_base *hb, const hashmap_entry *iter) {
    if (hb == NULL || hb->table == NULL || iter == NULL) return false;
    /* uintptr_t avoids undefined relational comparisons between unrelated objects. */
    uintptr_t first = (uintptr_t) hb->table;
    uintptr_t addr = (uintptr_t) iter;
    if (addr < first || addr - first >= hb->table_size * sizeof(*hb->table)) return false;
    if ((addr - first) % sizeof(*hb->table) != 0) return false;
    return iter->key != NULL;
}

bool hashmap_base_iter_next(const hashmap_base *hb, hashmap_entry **iter) {
    if (iter == NULL || *iter == NULL || !hashmap_base_iter_valid(hb, *iter)) return false;
    *iter = next_populated(hb, *iter + 1);
    return *iter != NULL;
}

bool hashmap_base_iter_remove(hashmap_base *hb, hashmap_entry **iter) {
    if (iter == NULL || *iter == NULL || !hashmap_base_iter_valid(hb, *iter)) return false;
    hashmap_entry *current = *iter;
    erase_slot(hb, current);
    *iter = next_populated(hb, current);
    return *iter != NULL;
}

const void *hashmap_base_iter_get_key(const hashmap_entry *iter) {
    return iter == NULL ? NULL : iter->key;
}

void *hashmap_base_iter_get_data(const hashmap_entry *iter) {
    return iter == NULL ? NULL : iter->data;
}

int hashmap_base_iter_set_data(hashmap_entry *iter, void *data) {
    if (iter == NULL) return -EFAULT;
    if (data == NULL) return -EINVAL;
    iter->data = data;
    return 0;
}

double hashmap_base_load_factor(const hashmap_base *hb) {
    return hb == NULL || hb->table_size == 0 ? 0.0 : (double) hb->size / (double) hb->table_size;
}

size_t hashmap_base_collisions(const hashmap_base *hb, const void *key) {
    if (hb == NULL || key == NULL || hb->table_size == 0) return 0;
    size_t index = initial_slot(hb, key);
    for (size_t probes = 0; probes < hb->table_size; ++probes) {
        const hashmap_entry *entry = &hb->table[index];
        if (entry->key == NULL) return 0;
        if (hb->compare(key, entry->key) == 0) return probes;
        index = next_slot(hb, index);
    }
    return 0;
}

double hashmap_base_collisions_mean(const hashmap_base *hb) {
    if (hb == NULL || hb->size == 0) return 0.0;
    double total = 0.0;
    for (size_t i = 0; i < hb->table_size; ++i) {
        if (hb->table[i].key != NULL) total += (double) hashmap_base_collisions(hb, hb->table[i].key);
    }
    return total / (double) hb->size;
}

double hashmap_base_collisions_variance(const hashmap_base *hb) {
    if (hb == NULL || hb->size == 0) return 0.0;
    double mean = hashmap_base_collisions_mean(hb);
    double total = 0.0;
    for (size_t i = 0; i < hb->table_size; ++i) {
        if (hb->table[i].key != NULL) {
            double deviation = (double) hashmap_base_collisions(hb, hb->table[i].key) - mean;
            total += deviation * deviation;
        }
    }
    return total / (double) hb->size;
}

/* Jenkins one-at-a-time hash, preserving the original size_t arithmetic. */
size_t hashmap_hash_default(const void *data, size_t length) {
    size_t hash = 0;
    const unsigned char *byte = (const unsigned char *) data;
    for (size_t i = 0; i < length; ++i) {
        hash += byte[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

size_t hashmap_hash_string(const char *key) {
    size_t hash = 0;
    for (const unsigned char *c = (const unsigned char *) key; *c != 0; ++c) {
        hash += *c;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

size_t hashmap_hash_string_i(const char *key) {
    size_t hash = 0;
    for (const unsigned char *c = (const unsigned char *) key; *c != 0; ++c) {
        hash += (size_t) tolower((int) *c);
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}
