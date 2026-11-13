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

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

typedef struct tagMTRand {
    uint32_t mt[STATE_VECTOR_LENGTH + 1];
    int32_t index;
} MTRand;

MTRand *seedRand(uint32_t seed);

uint32_t genRandLong(MTRand *rand);

double genRand(MTRand *rand);

#endif //MTWISTER_H
