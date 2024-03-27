//
// Created by aless on 22/03/2024.
//
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "commons.h"
#include "mtwister.h"

/*
 * TODO: fix memory leaks
 */
char* uuid_generate(void) {
    char* GUID = malloc(MAX_ID_SIZE);
    MTRand* seed = seedRand(clock());
    const char* szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    const int nLen = strlen(szTemp);
    for (int i = 0; i < nLen + 1; i++) {
        char c = szTemp[i];
        const char* szHex = "0123456789ABCDEF-";
        const uint32_t mt_num = genRandLong(seed) % 16;

        switch (c) {
            case 'x' : { c = szHex[mt_num]; } break;
            case 'y' : { c = szHex[mt_num & 0x03 | 0x08]; } break;
            default: break;
        }
        GUID[i] = i < nLen ? c : 0x00;
    }
    return GUID;
}
