/*
 * MT19937 vector conformance and historical-sequence regression (3.0.9).
 * Copyright (C) 2026 Alessandro Volpe
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Standard seed=5489 reference sequence: Matsumoto/Nishimura mt19937ar
 * and OEIS A221557 (also standardized by C++ std::mt19937).
 * Historical seed=1337 vectors: Evan Sultanik's public-domain MTwister
 * implementation, seedRand(1337), 6069 initialization.
 * Both generators must remain separately deterministic across platforms.
 */
#include "mtwister.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do {                                        \
    if (!(condition)) {                                               \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return EXIT_FAILURE;                                          \
    }                                                                 \
} while (0)

static int check_standard(void) {
    static const uint32_t expected_first_ten[] = {
        UINT32_C(3499211612), UINT32_C(581869302),
        UINT32_C(3890346734), UINT32_C(3586334585),
        UINT32_C(545404204), UINT32_C(4161255391),
        UINT32_C(3922919429), UINT32_C(949333985),
        UINT32_C(2715962298), UINT32_C(1323567403)
    };
    mtwister_t stack = {0};
    mtwister_seed_standard(&stack, UINT32_C(5489));
    mtwister_t *heap = mtwister_new(UINT32_C(5489));
    CHECK(heap != NULL);

    /* Compare heap and stack state through the first two twist blocks. */
    for (size_t i = 0; i < 626U; ++i) {
        const uint32_t value = mtwister_next_u32(&stack);
        CHECK(value == genRandLong(heap));
        if (i < sizeof expected_first_ten / sizeof expected_first_ten[0])
            CHECK(value == expected_first_ten[i]);
        if (i == 623U) CHECK(value == UINT32_C(4020325887));
        if (i == 624U) CHECK(value == UINT32_C(4178893912));
        if (i == 625U) CHECK(value == UINT32_C(610818241));
    }

    /* Reseeding must completely restore the sequence. */
    mtwister_seed_standard(&stack, UINT32_C(5489));
    CHECK(mtwister_next_u32(&stack) == expected_first_ten[0]);
    mtwister_destroy(heap);

    /* Seed 1 was independently cross-checked against C++ std::mt19937. */
    mtwister_seed_standard(&stack, UINT32_C(1));
    CHECK(mtwister_next_u32(&stack) == UINT32_C(1791095845));
    CHECK(mtwister_next_u32(&stack) == UINT32_C(4282876139));
    return EXIT_SUCCESS;
}

static int check_legacy(void) {
    static const uint32_t legacy_1337_first_ten[] = {
        UINT32_C(4087927374), UINT32_C(1462264401),
        UINT32_C(3976966677), UINT32_C(3309475782),
        UINT32_C(326292433), UINT32_C(2599138045),
        UINT32_C(1081378189), UINT32_C(3639934976),
        UINT32_C(2515071331), UINT32_C(844513513)
    };
    MTRand *legacy = seedRand(UINT32_C(1337));
    CHECK(legacy != NULL);
    for (size_t i = 0; i < 626U; ++i) {
        const uint32_t value = genRandLong(legacy);
        if (i < sizeof legacy_1337_first_ten / sizeof legacy_1337_first_ten[0])
            CHECK(value == legacy_1337_first_ten[i]);
        if (i == 623U) CHECK(value == UINT32_C(3116156308));
        if (i == 624U) CHECK(value == UINT32_C(2325197352));
        if (i == 625U) CHECK(value == UINT32_C(2576240916));
    }
    mtwister_destroy(legacy);

    /* The historical 6069 recurrence has a known zero-seed degeneracy.
     * It must not be silently mistaken for modern MT19937 behavior. */
    legacy = seedRand(0U);
    CHECK(legacy != NULL);
    for (unsigned i = 0; i < 626U; ++i) CHECK(genRandLong(legacy) == 0U);
    mtwister_destroy(legacy);
    return EXIT_SUCCESS;
}

static int check_double_contract(void) {
    mtwister_t *a = mtwister_new(UINT32_C(1337));
    mtwister_t *b = mtwister_new(UINT32_C(1337));
    CHECK(a != NULL && b != NULL);
    for (unsigned i = 0; i < 1000U; ++i) {
        const double expected = (double)mtwister_next_u32(a) / 4294967295.0;
        const double actual = mtwister_next_double(b);
        CHECK(actual == expected);
        CHECK(actual >= 0.0 && actual <= 1.0);
    }
    mtwister_destroy(a);
    mtwister_destroy(b);
    mtwister_destroy(NULL);
    return EXIT_SUCCESS;
}

int main(void) {
    if (check_standard() != EXIT_SUCCESS) return EXIT_FAILURE;
    if (check_legacy() != EXIT_SUCCESS) return EXIT_FAILURE;
    if (check_double_contract() != EXIT_SUCCESS) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
