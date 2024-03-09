//
// Created by aless on 08/03/2024.
//

#ifndef HASHMAP_BASE_H
#define HASHMAP_BASE_H
#include <stdbool.h>
#include <stddef.h>

typedef struct hashmap_entry {
    void* key;
    void* data;
} hashmap_entry;

typedef struct hashmap_base {
    size_t table_size_init;
    size_t table_size;
    size_t size;
    hashmap_entry* table;
    size_t (*hash)(const void*);
    int (*compare)(const void*, const void*);
    void *(*key_dup)(const void*);
    void (*key_free)(void*);
} hashmap_base;

void hashmap_base_init(hashmap_base* hb, size_t (*hash_func)(const void*),
    int (*compare_func)(const void*, const void*));
void hashmap_base_cleanup(hashmap_base* hb);


void hashmap_base_set_key_alloc_funcs(hashmap_base* hb,
    void* (*key_dup_func)(const void*), void (*key_free_func)(void*));

int hashmap_base_reserve(hashmap_base* hb, size_t capacity);

int hashmap_base_put(hashmap_base* hb, const void* key, void* data);
void* hashmap_base_get(const hashmap_base* hb, const void* key);
void* hashmap_base_remove(hashmap_base* hb, const void* key);

void hashmap_base_clear(hashmap_base* hb);
void hashmap_base_reset(hashmap_base* hb);

hashmap_entry* hashmap_base_iter(const hashmap_base* hb, const hashmap_entry* pos);
bool hashmap_base_iter_valid(const hashmap_base* hb, const hashmap_entry* iter);
bool hashmap_base_iter_next(const hashmap_base* hb, hashmap_entry** iter);
bool hashmap_base_iter_remove(hashmap_base* hb, hashmap_entry** iter);
const void* hashmap_base_iter_get_key(const hashmap_entry* iter);
void* hashmap_base_iter_get_data(const hashmap_entry* iter);
int hashmap_base_iter_set_data(hashmap_entry* iter, void* data);

double hashmap_base_load_factor(const hashmap_base *hb);
double hashmap_base_collisions_mean(const hashmap_base *hb);
double hashmap_base_collisions_variance(const hashmap_base *hb);

size_t hashmap_hash_default(const void *data, size_t len);
size_t hashmap_hash_string(const char *key);
size_t hashmap_hash_string_i(const char *key);
#endif //HASHMAP_BASE_H
