/*
 * LeaderboardSystem MT19937 generator (legacy).
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * MT19937 implementation adapted from Evan Sultanik's MTwister:
 * https://github.com/ESultanik/mtwister
 * The original author described the upstream work as public domain.
 * See THIRD_PARTY_NOTICES.md for provenance.
 */

#ifndef MTWISTER_H
#define MTWISTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

typedef struct tagMTRand {
    uint32_t mt[STATE_VECTOR_LENGTH + 1];
    int32_t index;
} MTRand;

/* Modern alias; the historical MTRand layout and name remain unchanged. */
typedef MTRand mtwister_t;

/* Historical Evan Sultanik initialization (multiplier 6069). Returns a
 * heap-owned generator or NULL on allocation failure. The sequence is NOT
 * the same as std::mt19937 for the same seed, including seed 5489. */
MTRand *seedRand(uint32_t seed);

/* Standard MT19937 initialization (Matsumoto/Nishimura 2002), matching
 * std::mt19937 seeded by the same 32-bit integer. Call seed_standard on
 * caller-owned storage, or use new for a heap-owned instance. */
void mtwister_seed_standard(mtwister_t *rng, uint32_t seed);
mtwister_t *mtwister_new(uint32_t seed);

/* Frees only heap-owned instances (seedRand or mtwister_new); NULL is safe.
 * Do not use with stack-allocated mtwister_t objects. */
void mtwister_destroy(mtwister_t *rng);

/* Both algorithms share an MT19937 twist/temper step, returning all 32 bits.
 * rng must be a valid seeded instance, not NULL. */
uint32_t mtwister_next_u32(mtwister_t *rng);

/* Returns a closed-interval [0, 1] double, integer / UINT32_MAX. */
double mtwister_next_double(mtwister_t *rng);


/* Legacy generator functions retained with their existing semantics. */
uint32_t genRandLong(MTRand *rand);

double genRand(MTRand *rand);

#ifdef __cplusplus
}
#endif

#endif //MTWISTER_H
