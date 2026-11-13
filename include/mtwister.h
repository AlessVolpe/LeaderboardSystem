/*
 * MT19937 implementation adapted from Evan Sultanik's MTwister:
 * https://github.com/ESultanik/mtwister
 * The original author described the upstream work as public domain.
 * Modifications Copyright (C) 2024-2026 Alessandro Volpe.
 * See THIRD_PARTY_NOTICES.md for provenance.
 */

//
// Created by aless on 21/03/2024.
//

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
