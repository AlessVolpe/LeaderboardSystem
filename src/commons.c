/*
 * LeaderboardSystem UUID utilities (legacy).
 * Copyright (C) 2024-2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "commons.h"
#include "mtwister.h"

/* Returned UUID is heap-allocated and belongs to the caller. */
char *uuid_generate(void) {
    static uint32_t sequence = 0;
    char *GUID = malloc(MAX_ID_SIZE);
    /* Mix a per-process sequence to avoid identical seeds on the same clock tick. */
    MTRand *seed = seedRand((uint32_t)clock() ^ (++sequence * UINT32_C(0x9E3779B9)));
    if (GUID == NULL || seed == NULL) {
        free(GUID);
        free(seed);
        return NULL;
    }
    const char *szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    const int nLen = strlen(szTemp);
    for (int i = 0; i < nLen + 1; i++) {
        char c = szTemp[i];
        const char *szHex = "0123456789ABCDEF-";
        const uint32_t mt_num = genRandLong(seed) % 16;

        switch (c) {
            case 'x': { c = szHex[mt_num]; }
            break;
            case 'y': { c = szHex[(mt_num & 0x03) | 0x08]; }
            break;
            default: break;
        }
        GUID[i] = i < nLen ? c : 0x00;
    }
    free(seed);
    return GUID;
}
