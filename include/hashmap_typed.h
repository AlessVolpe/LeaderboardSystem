/*
 * LeaderboardSystem portable typed hashmap adapters (milestone 3.0.7).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Uses the hashmap originally developed by David Leeds (MIT).
 * See THIRD_PARTY_NOTICES.md for full upstream licensing information.
 */
#ifndef LEADERBOARD_HASHMAP_TYPED_H
#define LEADERBOARD_HASHMAP_TYPED_H

#include "hashmap_base.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/* Optional diagnostic suppression, not used for callback calling conventions.
 * MSVC and other compilers receive ordinary ISO C11 inline definitions. */
#if defined(__GNUC__) || defined(__clang__)
#define LEADERBOARD_TYPED_UNUSED __attribute__((unused))
#else
#define LEADERBOARD_TYPED_UNUSED
#endif

/*
 * Declare a named, C11-compatible typed hashmap at file scope:
 *
 *   HASHMAP_TYPED_DEFINE(scores, char, int)
 *   scores_t map = {0};
 *   int rc = scores_init(&map, hashmap_hash_string, strcmp);
 *   ... scores_put(&map, "alice", &score); ... scores_cleanup(&map);
 *
 * Key and value types are the pointees; the API takes their pointers.
 * No function-pointer casts, typeof, statement expressions or zero-length
 * arrays are involved. The adapter context is heap-owned by the hashmap.
 * All values are borrowed; keys are borrowed until ownership callbacks
 * are configured. Maps must not be copied or moved after initialization.
 * This macro must be invoked at file scope, not inside a function.
 */
#define HASHMAP_TYPED_DEFINE(name, key_type, value_type)                                      \
    typedef struct name##_callbacks {                                                        \
        size_t (*hash)(const key_type *);                                                      \
        int (*compare)(const key_type *, const key_type *);                                   \
        key_type *(*duplicate)(const key_type *);                                              \
        void (*release)(key_type *);                                                          \
    } name##_callbacks_t;                                                                    \
    typedef struct name {                                                                     \
        hashmap_base_t *base;                                                                 \
        name##_callbacks_t *callbacks;                                                        \
    } name##_t;                                                                               \
    typedef struct name##_iterator {                                                          \
        hashmap_base_t *base;                                                                 \
        hashmap_entry_t *entry;                                                               \
    } name##_iterator_t;                                                                      \
    static size_t name##_hash_adapter(const void *key, void *context) {                        \
        const name##_callbacks_t *cb = context;                                               \
        return cb->hash((const key_type *)key);                                                \
    }                                                                                        \
    static int name##_compare_adapter(const void *a, const void *b, void *context) {           \
        const name##_callbacks_t *cb = context;                                               \
        return cb->compare((const key_type *)a, (const key_type *)b);                           \
    }                                                                                        \
    static void *name##_duplicate_adapter(const void *key, void *context) {                   \
        const name##_callbacks_t *cb = context;                                               \
        return cb->duplicate((const key_type *)key);                                           \
    }                                                                                        \
    static void name##_release_adapter(void *key, void *context) {                            \
        const name##_callbacks_t *cb = context;                                               \
        cb->release((key_type *)key);                                                          \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED int name##_init(name##_t *map,                                              \
            size_t (*hash)(const key_type *),                                                  \
            int (*compare)(const key_type *, const key_type *)) {                              \
        if (map == NULL || hash == NULL || compare == NULL) return -EINVAL;                    \
        map->base = NULL;                                                                     \
        map->callbacks = NULL;                                                                \
        name##_callbacks_t *ctx = calloc(1, sizeof(*ctx));                                     \
        if (ctx == NULL) return -ENOMEM;                                                       \
        ctx->hash = hash;                                                                     \
        ctx->compare = compare;                                                               \
        hashmap_base_t *base = hashmap_base_new_with_context(                                  \
            name##_hash_adapter, name##_compare_adapter, ctx, free);                           \
        if (base == NULL) { free(ctx); return -ENOMEM; }                                       \
        map->base = base;                                                                     \
        map->callbacks = ctx;                                                                 \
        return 0;                                                                             \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED void name##_cleanup(name##_t *map) {                                       \
        if (map == NULL) return;                                                              \
        hashmap_base_destroy(map->base);                                                      \
        map->base = NULL;                                                                     \
        map->callbacks = NULL;                                                                \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED int name##_set_key_alloc_funcs(name##_t *map,                               \
            key_type *(*duplicate)(const key_type *),                                          \
            void (*release)(key_type *)) {                                                     \
        if (map == NULL || map->base == NULL || map->callbacks == NULL) return -EINVAL;        \
        if ((duplicate == NULL) != (release == NULL)) return -EINVAL;                          \
        int rc = hashmap_base_set_context_key_alloc_funcs(map->base,                          \
            duplicate ? name##_duplicate_adapter : NULL,                                     \
            release ? name##_release_adapter : NULL);                                         \
        if (rc == 0) {                                                                        \
            map->callbacks->duplicate = duplicate;                                            \
            map->callbacks->release = release;                                                \
        }                                                                                    \
        return rc;                                                                            \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED size_t name##_size(const name##_t *map) {                                  \
        return map == NULL ? 0u : hashmap_base_size(map->base);                                \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED int name##_reserve(name##_t *map, size_t capacity) {                        \
        return map == NULL ? -EINVAL : hashmap_base_reserve(map->base, capacity);              \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED int name##_put(name##_t *map, const key_type *key, value_type *value) {      \
        if (map == NULL) return -EINVAL;                                                      \
        return hashmap_base_put(map->base, key, value);                                        \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED value_type *name##_get(const name##_t *map, const key_type *key) {           \
        return map == NULL ? NULL : (value_type *)hashmap_base_get(map->base, key);            \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED value_type *name##_remove(name##_t *map, const key_type *key) {              \
        return map == NULL ? NULL : (value_type *)hashmap_base_remove(map->base, key);         \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED void name##_clear(name##_t *map) {                                         \
        if (map != NULL) hashmap_base_clear(map->base);                                       \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED void name##_reset(name##_t *map) {                                         \
        if (map != NULL) hashmap_base_reset(map->base);                                       \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED name##_iterator_t name##_iter(name##_t *map) {                              \
        name##_iterator_t it = {NULL, NULL};                                                  \
        if (map != NULL) {                                                                    \
            it.base = map->base;                                                              \
            it.entry = hashmap_base_iter(it.base, NULL);                                      \
        }                                                                                    \
        return it;                                                                            \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED bool name##_iter_valid(const name##_iterator_t *it) {                       \
        return it != NULL && hashmap_base_iter_valid(it->base, it->entry);                    \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED bool name##_iter_next(name##_iterator_t *it) {                              \
        return it != NULL && hashmap_base_iter_next(it->base, &it->entry);                     \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED bool name##_iter_remove(name##_iterator_t *it) {                            \
        return it != NULL && hashmap_base_iter_remove(it->base, &it->entry);                   \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED const key_type *name##_iter_key(const name##_iterator_t *it) {               \
        return it == NULL ? NULL : (const key_type *)hashmap_base_iter_get_key(it->entry);    \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED value_type *name##_iter_value(const name##_iterator_t *it) {                 \
        return it == NULL ? NULL : (value_type *)hashmap_base_iter_get_data(it->entry);       \
    }                                                                                        \
    static inline LEADERBOARD_TYPED_UNUSED int name##_iter_set_value(name##_iterator_t *it, value_type *value) {        \
        return it == NULL ? -EINVAL : hashmap_base_iter_set_data(it->entry, value);            \
    }

#endif /* LEADERBOARD_HASHMAP_TYPED_H */
