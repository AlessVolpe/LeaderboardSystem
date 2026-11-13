# 3.0.9: PRNG reproducibility and portable integration

## Version contract

`project(leaderboardsystem VERSION 3.0.9 LANGUAGES C)` is the authoritative
package version. CMake writes `LeaderboardConfigVersion.cmake` from that
version, and an independent example requires `find_package(Leaderboard 3.0.9
CONFIG REQUIRED)` before linking `Leaderboard::leaderboard`. Starting with
this milestone, the source version changes in each numbered milestone.

The project follows its own milestone numbering convention within the `3.x`
series. CMake versions still use numeric `major.minor.patch` components; the
version is not a decimal or a base-12 number.

## Two explicitly different MT19937 seeding contracts

The existing `seedRand(seed)` is retained with exactly its historical behavior.
It returns an allocated `MTRand *`, initializes `mt[0] = seed` and the remaining
623 active state words using unsigned 32-bit arithmetic:

```
mt[i] = 6069 * mt[i - 1] (mod 2^32)
```

This is the seeding algorithm in Evan Sultanik's upstream `MTwister` C library,
not the initialization of modern C++ `std::mt19937`. The historical generator
is *not* standards-equivalent even when the same seed is supplied; a seed of
zero is degenerate and produces a stream of zeros. Historical output is frozen
by regression vectors for seed 1337 across the 624/625-word twist boundary.
**Do not switch existing users silently to the standard constructor.**

`mtwister_new(seed)` or `mtwister_seed_standard(&state, seed)` instead use the
Matsumoto/Nishimura 2002 initialization, matching a 32-bit-seeded
`std::mt19937`:

```
mt[0] = seed
mt[i] = 1812433253 * (mt[i-1] ^ (mt[i-1] >> 30)) + i (mod 2^32)
```

Both modes use the same MT19937 state-transition and tempering algorithm.
`mtwister_next_u32` (and historical `genRandLong`) produce one `uint32_t` per
call, spanning the whole `[0, UINT32_MAX]` range. `mtwister_next_double`
(and historical `genRand`) divide that integer by 4294967295.0, yielding the
**closed** interval `[0, 1]`, not the half-open interval some applications
expect. Floating-point values are deterministic on implementations with the
same standard IEEE-compatible `double` behavior; the 32-bit integer stream is
the primary interoperability contract.

* Identical seed + algorithm + output count gives an identical stream.
* Re-seeding a caller-owned state resets its sequence.
* `mtwister_new` and `seedRand` return heap storage, owned by the caller;
  release either with `mtwister_destroy` (or the historical `free`).
* `mtwister_seed_standard` can initialize a stack-allocated `mtwister_t`;
  **do not free stack storage** with `mtwister_destroy`.
* Independent generator objects have independent states; sharing one object
  between threads without synchronization is unsupported.
* The historical public `MTRand` layout remains visible for compatibility,
  but clients should not edit `mt[]` or `index` directly.
* Neither mode is a cryptographically secure PRNG. In particular,
  `uuid_generate()` uses this legacy family and is not suitable for secure
  tokens, secrets, or unpredictable identifier generation.

### Reference sources

* [Evan Sultanik's MTwister (upstream code)](https://github.com/ESultanik/mtwister/blob/master/mtwister.c)
  for the historical seeding recurrence, twist and tempering.
* [Matsumoto & Nishimura, original MT19937 algorithm (1998)](https://doi.org/10.1145/272991.272995).
* [Matsumoto & Nishimura, 2002 initialization (`mt19937ar`)](https://sources.debian.org/src/orc/1%3A0.4.32-1/examples/mt19937ar.c).
* [OEIS A221557](https://oeis.org/A221557), the published standard MT19937
  output vector for default seed 5489. The first values are 3499211612,
  581869302, 3890346734, 3586334585, 545404204.

`tests/test_mtwister_vectors.c` checks the published seed-5489 sequence,
independently verified seed-1 and twist-boundary values, legacy seed-1337
vectors, re-seeding, and double normalization. Standard boundary values were
also checked independently against the C++ `std::mt19937` implementation.

## Naming and portable interfaces

Use the consistently suffixed opaque public type names in new programs:

| Native API | Preferred type | Historical name still supported |
|---|---|---|
| Ranking | `ranking_t` | N/A |
| Hashmap base | `hashmap_base_t` | N/A; old embedded base is no longer public |
| BOSTree | `bostree_t` / `bostree_node_t` | `BOSTree` / `BOSNode` |
| MT19937 | `mtwister_t` | `MTRand` |
| Legacy player | `player_t` | Already consistently suffixed |

The BOSTree aliases already existed before 3.0.9. The `mtwister_t` alias is
additive and has the same layout and ABI as `MTRand`. No existing public symbol
has been removed or renamed; legacy `seedRand`, `genRandLong`, and `genRand`
keep their prior output sequences and ownership rules.

A `player_t` has an unsigned 32-bit score and borrows its name string;
`ranking_t` owns copied identifiers and uses signed 64-bit scores. This is a
semantic boundary, not just a naming difference. No implicit conversion is
introduced.

New portable hashmaps should use `HASHMAP_TYPED_DEFINE` in
`include/hashmap_typed.h` and the C11-compatible functions in
`include/hashmap_base.h`. The deprecated `HASHMAP(...)` family in
`include/hashmap.h` still depends on GNU `typeof` and statement expressions;
those are preserved for legacy GCC/Clang users and are **not** ISO C11 APIs.

New code can compile with `CMAKE_C_EXTENSIONS=OFF`; the shipped library
compiles as C11. Dedicated public-header compile probes use strict ISO C11
with `-pedantic-errors` when supported. The `commons.h` error macros now
explicitly include `<string.h>` for their `strerror` declaration.

## Independent integration and example

`examples/ranking_batch.c` uses only `ranking.h` and standard C11 headers.
It demonstrates a deterministic, non-interactive score update, tie breaking,
rank selection, removal and borrowed identifier queries; its executable is
`leaderboard_ranking_batch`, and `ranking_batch_example` runs in CTest.

For a clean external installation (example POSIX shell):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DLEADERBOARD_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/installed"
cmake -S examples/external_consumer -B build-consumer -G Ninja \
  -DCMAKE_PREFIX_PATH="$PWD/installed"
cmake --build build-consumer
ctest --test-dir build-consumer --output-on-failure
```

The external consumer checks `ranking_t`, portable hashmap callbacks, modern
`mtwister_t`, and `find_package(Leaderboard 3.0.9 CONFIG REQUIRED)` without
using `src/internal`. The `installed_consumer` CTest performs an isolated
install, configure, build and test automatically on supported CI toolchains.

### Explicit limitations

* The deprecated GNU hashmap macros are intentionally not rewritten in 3.0.9.
* `MTRand` preserves its old public representation; it is not an opaque type.
* No claim of C++ ABI beyond the `extern "C"` functions in `mtwister.h`.
* Host-specific `clock()` seeding and legacy UUID generation are not
  cross-process reproducibility or secure-identity guarantees.
