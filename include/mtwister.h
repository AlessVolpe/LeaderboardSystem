//
// Created by aless on 21/03/2024.
//

#ifndef MTWISTER_H
#define MTWISTER_H

#include <stdint.h>

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

typedef struct tagMTRand {
    uint32_t mt[STATE_VECTOR_LENGTH+1];
    int32_t index;
} MTRand;

MTRand* seedRand(uint32_t seed);
uint32_t genRandLong(MTRand* rand);
double genRand(MTRand* rand);

#endif //MTWISTER_H
