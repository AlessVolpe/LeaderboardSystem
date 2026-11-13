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

#include "mtwister.h"

#include <stdlib.h>

#define UPPER_MASK		    0x80000000
#define LOWER_MASK		    0x7fffffff
#define TEMPERING_MASK_B	0x9d2c5680
#define TEMPERING_MASK_C	0xefc60000

/*
 * set initial seeds to mt[STATE_VECTOR_LENGTH using the generator
 * illustrated at Line 25 of Table 1 in Donald Knuth's "The Art of Computer Programming"
 * Vol. 2 (2nd Ed.) pp.102.
 */
static void m_seedRand(MTRand *rand, const uint32_t seed) {
    if (rand) {
        rand->mt[0] = seed & 0xffffffff;
        for (rand->index = 1; rand->index < STATE_VECTOR_LENGTH; rand->index++)
            rand->mt[rand->index] = 6069 * rand->mt[rand->index - 1] & 0xffffffff;
    }
}

/*
 * creates a new random number generator from a given seed
 * The caller owns the allocated state and must free it.
 */
/* Uses the MT19937 2002 single-word seeding recurrence. Unlike seedRand,
 * this has a published, cross-language reference output sequence. */
void mtwister_seed_standard(mtwister_t *rng, const uint32_t seed) {
    if (rng == NULL) return;
    rng->mt[0] = seed;
    for (uint32_t i = 1; i < STATE_VECTOR_LENGTH; ++i) {
        const uint32_t previous = rng->mt[i - 1U];
        rng->mt[i] = UINT32_C(1812433253) * (previous ^ (previous >> 30)) + i;
    }
    rng->index = STATE_VECTOR_LENGTH;
}

mtwister_t *mtwister_new(const uint32_t seed) {
    mtwister_t *rng = malloc(sizeof(*rng));
    if (rng != NULL) mtwister_seed_standard(rng, seed);
    return rng;
}

void mtwister_destroy(mtwister_t *rng) {
    free(rng);
}

MTRand *seedRand(const uint32_t seed) {
    MTRand *rand = malloc(sizeof(MTRand));
    m_seedRand(rand, seed);
    return rand;
}

/*
 * generates a pseudo-random 32-bit integer
 */
uint32_t genRandLong(MTRand *rand) {
    uint32_t y;
    static uint32_t mag[2] = {0x0, 0x9908b0df}; // mag[x] = x * 0x9908b0df for x = 0, 1
    if (rand->index >= STATE_VECTOR_LENGTH || rand->index < 0) {
        // generate STATE_VECTOR_LENGTH words a time
        if (rand->index >= STATE_VECTOR_LENGTH + 1 || rand->index < 0) m_seedRand(rand, 4357);
        int32_t kk;
        for (kk = 0; kk < STATE_VECTOR_LENGTH - STATE_VECTOR_M; kk++) {
            y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
            rand->mt[kk] = rand->mt[kk + STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
        }

        for (; kk < STATE_VECTOR_LENGTH - 1; kk++) {
            y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
            rand->mt[kk] = rand->mt[kk + (STATE_VECTOR_M - STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
        }
        y = (rand->mt[STATE_VECTOR_LENGTH - 1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
        rand->mt[STATE_VECTOR_LENGTH - 1] = rand->mt[STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
        rand->index = 0;
    }

    y = rand->mt[rand->index++];
    y ^= y >> 11;
    y ^= y << 7 & TEMPERING_MASK_B;
    y ^= y << 15 & TEMPERING_MASK_C;
    y ^= y >> 18;
    return y;
}

/*
 * generates a pseudo random double in the range [0, 1]
 */
double genRand(MTRand *rand) {
    return (double) genRandLong(rand) / 0xffffffff;
}

/* Named aliases for new clients; they intentionally use the same algorithm
 * as the corresponding historical generation functions. */
uint32_t mtwister_next_u32(mtwister_t *rng) {
    return genRandLong(rng);
}

double mtwister_next_double(mtwister_t *rng) {
    return genRand(rng);
}
