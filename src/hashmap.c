//
// Created by aless on 08/03/2024.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "commons.h"
#include "hashmap_base.h"

#define HASHMAP_SIZE_MIN                32
#define HASHMAP_SIZE_DEFAULT            128
#define HASHMAP_SIZE_MOD(map, val)      ((val) & ((map)->table_size - 1))

#define HASHMAP_PROBE_NEXT(map, index)  HASHMAP_SIZE_MOD(map, (index) + 1)


/*
* Calculate the optimal table size, given the specified max number
* of elements.
*/
static size_t hashmap_calc_table_size(const hashmap_base* hb, const size_t size) {
    // Enforce a maximum 0.75 load factor
    size_t table_size = size + (size / 3);

    // size shouldn't be lower than inizial size
    if (table_size < hb->table_size_init) table_size = hb->table_size_init;
    // round table size up to the nearest power of 2
    else table_size = 1 << ((sizeof(unsigned long) << 3) - __builtin_clzl(table_size - 1));
    return table_size;
}

/*
 * Get a valid hash table index from a key.
 */
static size_t hashmap_calc_index(const hashmap_base* hb, const void* key) {
    size_t index = hb->hash(key);

    // run a secondary hash on the index, small performance hit but improves consinstency
    index = hashmap_hash_default(&index, sizeof(index));
    return HASHMAP_SIZE_MOD(hb, index);
}

/*
 * Return the next populated entry, starting with the specified one.
 * Returns NULL if there are no more valid entries.
 */
static hashmap_entry* hashmap_entry_get_populated(const hashmap_base* hb, const hashmap_entry* entry) {
    if (hb->size > 0) {
        for(; entry < &hb->table[hb->table_size]; ++entry) {
            if (entry->key) return (hashmap_entry*) entry;
        }
    }
    return NULL;
}


/*
 * Find the hashmap entry with the specified key, or an empty slot.
 * Returns NULL if the entire table has been searched without finding a match.
 */
static hashmap_entry* hashmap_entry_find(const hashmap_base* hb, const void* key, const bool find_empty) {
    size_t index = hashmap_calc_index(hb, key);

    // linear probing
    for (size_t i = 0; i < hb->table_size; ++i) {
        hashmap_entry* entry = &hb->table[index];
        if (entry->key == NULL) {
            if (find_empty) return entry;
            return NULL;
        }

        if (hb->compare(key, entry->key) == 0) return entry;
        index = HASHMAP_PROBE_NEXT(hb, index);
    }
    return NULL;
}

/*
 * Removes the specified entry and processes the following entries to
 * keep the chain contiguous.
 */
static void hashmap_entry_remove(hashmap_base* hb, hashmap_entry* removed_entry) {
    size_t removed_index = (removed_entry - hb->table);

    // free the key
    if (hb->key_free) hb->key_free(removed_entry->key);
    --hb->size;

    // fill the free slot in the chain
    size_t index = HASHMAP_PROBE_NEXT(hb, removed_index);
    for (size_t i = 0; i < hb->size; ++i) {
        hashmap_entry* entry = &hb->table[index];
        if (entry->key == NULL) break; // reached end of the chain

        const size_t entry_index = hashmap_calc_index(hb, entry->key);
        // shift in the chain entries with an index at or before the removed slot
        if (HASHMAP_SIZE_MOD(hb, index - entry_index) > HASHMAP_SIZE_MOD(hb, removed_index - entry_index)) {
            *removed_entry = *entry;
            removed_index = index;
            removed_entry = entry;
        }
        index = HASHMAP_PROBE_NEXT(hb, index);
    }
    // clear the last removed entry
    memset(removed_entry, 0, sizeof(*removed_entry));
}

/*
 * Reallocates the hash table to the new size and rehashes all entries.
 * new_size MUST be a power of 2.
 * Returns 0 on success and -errno on allocation or hash function failure.
 */
static int hashmap_rehash(hashmap_base* hb, const size_t table_size) {
    assert((table_size & table_size - 1) == 0);
    assert(table_size >= hb->size);

    hashmap_entry* new_table = calloc(table_size, sizeof(hashmap_entry));
    ERROR_RETURN((new_table == NULL), -ENOMEM, "REHASH_ERR: not enough memory");

    const size_t old_size = hb->table_size;
    hashmap_entry* old_table = hb->table;
    hb->table_size = table_size;
    hb->table = new_table;

    if (old_table == NULL) return 0;

    // rehash
    for (const hashmap_entry* entry = old_table; entry < &old_table[old_size]; ++entry) {
        if (entry->key == NULL) continue;
        hashmap_entry* new_entry = hashmap_entry_find(hb, entry->key, true);
        assert(new_entry != NULL);

        // shallow copy
        *new_entry = *entry;
    }
    free(old_table);
    return 0;
}

/*
 * Iterate through all entries and free all keys.
 */
static void hashmap_free_keys(const hashmap_base* hb) {
    GENERIC_ERROR_HELPER((hb->key_free == NULL || hb->size == 0),
        errno, "FREE_KEYS_ERR: full hashmap or no keys to free");

    for (const hashmap_entry* entry = hb->table; entry < &hb->table[hb->table_size]; ++entry)
        if (entry->key) hb->key_free(entry->key);
}

/*
 * Initialize an empty hashmap.
 *
 * hash_func should return an even distribution of numbers between 0
 * and SIZE_MAX varying on the key provided.
 *
 * compare_func should return 0 if the keys match, and non-zero otherwise.
 */
void hashmap_base_init(hashmap_base* hb, size_t (*hash_func)(const void*),
    int (*compare_func)(const void*, const void*)) {
    assert(hash_func != NULL);
    assert(compare_func != NULL);

    memset(hb, 0, sizeof(*hb));

    hb->table_size_init = HASHMAP_SIZE_DEFAULT;
    hb->hash = hash_func;
    hb->compare = compare_func;
}


/*
 * Free the hashmap and all associated memory.
 */
void hashmap_base_cleanup(hashmap_base* hb) {
    GENERIC_ERROR_HELPER((hb == NULL), errno, "CLEANUP_ERR: hashmap doesn't exist");
    hashmap_free_keys(hb);
    free(hb->table);
    memset(hb, 0, sizeof(*hb));
}


/*
 * Enable internal memory management of hash keys.
 */
void hashmap_base_set_alloc_funcs(hashmap_base* hb, void* (*key_dup_func)(const void*), void (*key_free_func)(void*)) {
    hb->key_dup = key_dup_func;
    hb->key_free = key_free_func;
}

/*
 * Set the hashmap's initial allocation size such that no rehashes are
 * required to fit the specified number of entries.
 * Returns 0 on success, or -errno on failure.
 */
int hashmap_base_reserve(hashmap_base* hb, const size_t capacity) {
    int r = 0;
    const size_t old_size_init = hb->table_size_init; // original size backup

    // set minimal table init size to support the specified capacity
    hb->table_size_init = HASHMAP_SIZE_MIN;
    hb->table_size_init = hashmap_calc_table_size(hb, capacity);

    if (hb->table_size_init > hb->table_size) {
        r = hashmap_rehash(hb, hb->table_size_init);
        if (r < 0) hb->table_size_init = old_size_init;
    }
    return r;
}

/*
 * Add a new entry to the hashmap. If an entry with a matching key
 * already exists -EEXIST is returned.
 * Returns 0 on success, or -errno on failure.
 */
int hashmap_base_put(hashmap_base* hb, const void* key, const void* data) {
    int r = 0;
    ERROR_RETURN((key == NULL || data == NULL), -EINVAL, "PUT_ERR: null key or data parameters");

    // preemptively rehash with 2x capacity if load factor is approacing 0.75
    const size_t table_size = hashmap_calc_table_size(hb, hb->size);
    if (table_size > hb->table_size) r = hashmap_rehash(hb, table_size);

    // get the entry for this key
    hashmap_entry* entry = hashmap_entry_find(hb, key, true);
    ERROR_RETURN((entry != NULL), -EADDRNOTAVAIL, "PUT_ERR: memory address not valid");
    if (r < 0) return r; // rehash error

    ERROR_RETURN(entry->key, -EEXIST, "PUT_ERR: key already exists");
    if (hb->key_dup == NULL) {
        entry->key = (void*) key;
        entry->data = (void*) data;
        ++hb->size;
    } else {
        // allocate copy of key to simplify memory management
        entry->key = hb->key_dup(key);
        ERROR_RETURN((entry->key == NULL), -ENOMEM, "PUT_ERR: not enough memory");
    }
    return 0;
}

/*
 * Return the data pointer, or NULL if no entry exists.
 */
void* hashmap_base_get(const hashmap_base* hb, const void* key) {
    if (key == NULL) return NULL;

    const hashmap_entry* entry = hashmap_entry_find(hb, key, false);
    if (entry == NULL) return NULL;
    return entry->data;
}

/*
 * Remove an entry with the specified key from the map.
 * Returns the data pointer, or NULL, if no entry was found.
 */
void* hahsmap_base_remove(hashmap_base* hb, const void* key) {
    if (key == NULL) return NULL;

    hashmap_entry* entry = hashmap_entry_find(hb, key, false);
    if (entry == NULL) return NULL;
    void* data = entry->data;

    // remove entry and make chain contiguos
    hashmap_entry_remove(hb, entry);
    return data;
}

/*
 * Remove all entries.
 */
void hashmap_base_clear(hashmap_base* hb) {
    hashmap_free_keys(hb);
    hb->size = 0;
    memset(hb->table, 0, sizeof(hashmap_entry) * hb->table_size);
}

/*
 * Remove all entries and reset the hash table to its initial size.
 */
void hashmap_base_reset(hashmap_base* hb) {

    hashmap_free_keys(hb);
    hb->size = 0;

    if (hb->table_size != hb->table_size_init) {
        hashmap_entry* new_table = realloc(hb->table, sizeof(hashmap_entry*) * hb->table_size_init);
        if (new_table != NULL) {
            hb->table = new_table;
            hb->table_size = hb->table_size_init;
        }
    }
    memset(hb->table, 0, sizeof(hashmap_entry) * hb->table_size);
}

/*
 * Get a new hashmap iterator. The iterator is an opaque
 * pointer that may be used with hashmap_iter_*() functions.
 * Hashmap iterators are INVALID after a put or remove operation is performed.
 * hashmap_iter_remove() allows safe removal during iteration.
 */
hashmap_entry* hashmap_base_iter(const hashmap_base* hb, const hashmap_entry* pos) {
    if (pos == NULL) pos = hb->table;
    return hashmap_entry_get_populated(hb, pos);
}

/*
 * Return true if an iterator is valid and safe to use.
 */
bool hashmap_base_iter_valid(const hashmap_base* hb, const hashmap_entry* iter) {
    return hb && iter && iter->key && iter >= hb->table && iter < &hb->table[hb->table_size];
}

/*
 * Advance an iterator to the next hashmap entry.
 * Returns false if there are no more entries.
 */
bool hashmap_base_iter_next(const hashmap_base* hb, hashmap_entry** iter) {
    if (*iter == NULL) return false;
    return (*iter = hashmap_entry_get_populated(hb, *iter + 1)) != NULL;
}