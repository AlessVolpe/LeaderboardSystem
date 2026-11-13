# Portable callbacks, failure injection and installation (3.0.7)

Milestone 3.0.7 extends the encapsulated 3.0.6 native library without changing
hashmap collision resolution, BOSTree balancing or the generic ranking order.
It introduces a strict-C11 route for typed hashmap consumers, deterministic
allocation-failure tests, and a configurable CMake install package.

## Type-correct callback adapters

`include/hashmap_typed.h` exposes the file-scope macro
`HASHMAP_TYPED_DEFINE(name, key_type, value_type)`. It generates the named
`name_t` and `name_iterator_t` types plus `name_init`, `name_cleanup`,
`name_set_key_alloc_funcs`, `name_put`, `name_get`, `name_remove`, `name_size`,
`name_reserve`, `name_clear`, `name_reset` and iterator functions.

```c
#include <string.h>
#include "hashmap_typed.h"
HASHMAP_TYPED_DEFINE(scores, char, int)

scores_t map = {0};
if (scores_init(&map, hashmap_hash_string, strcmp) != 0) { /* failure */ }
int score = 42;
if (scores_put(&map, "alice", &score) != 0) { /* failure */ }
scores_cleanup(&map);
```

The named wrapper retains exactly typed function pointers for hashing,
comparison, optional key duplication and release. Static adapters accept
`const void *` and context pointers with the *actual* generic callback
signatures; they cast only **object pointers** back to the declared key type.
The hashmap holds a separately allocated callback context until destruction.
This avoids the incompatible-function-pointer casts in the old `HASHMAP`
macros, while adding no per-entry boxing allocation. This API requires a
zero-initialized map before first initialization; do not copy an initialized
map, initialize it twice, or use its iterators after structural mutations.

When owning key copies, call `name_set_key_alloc_funcs` on an empty map with
a complete duplicate/release pair. Borrowed keys must remain alive for the
lifetime of their map entries. Values remain borrowed in all cases. Key
callbacks are frozen after insertion. `name_cleanup` releases keys, the hash
table, and the callback context; it does not free values. The generated
callbacks use standard C11 functions, structs and inline functions; the
conditionally enabled compiler `unused` attribute only suppresses unused
helper warnings and is not required for correctness.

The public low-level `hashmap_base_new_with_context` supports generic
context-aware hash/comparison callbacks; optional owned-key hooks use
`hashmap_base_set_context_key_alloc_funcs`. Construction takes ownership of
context only on success when a destructor is supplied; on construction failure
the caller still owns it. Context destruction happens after all key finalizers.

The legacy `HASHMAP` interface is retained for source migration. It still
requires GNU `typeof` and statement expressions and still casts unrelated
function pointer types, which is not strictly conforming C. New applications
should use the named API. The sample CLI moves to the C11 API; the legacy
macro regression tests remain to exercise compatibility behavior.

## Allocation failure testing

`src/internal/alloc_internal.h` provides the internal allocation boundary
used by the hashmap, BOSTree and ranking implementations. Production calls
forward to `malloc`/`calloc`. A separate **test-only** `leaderboard_fault_test`
archive is compiled with `LEADERBOARD_TEST_ALLOCATION_FAULTS`; the private
`leaderboard_alloc_fail_after(n)` hook causes the (n+1)-th allocation to fail
once, while `leaderboard_alloc_reset()` disables faults.

`allocation_failures` covers construction, first hashmap allocation, new
ranking entries and score-update rollback. It checks membership, score/rank,
size and removal after failures. The control is intentionally process-global
and single-threaded, and does not intercept allocations in legacy PRNG, UUID,
libc callbacks or user code. These tests provide reproducible fault cases,
not a proof of complete failure atomicity; stress fuzzing and allocator
matrix expansion remain future work. Only the fault test archive exports
injection control symbols, not the production static library.

## Build and external consumers

Native library sources are compiled with C extensions disabled. Legacy C
macro tests enable GNU extensions on their individual targets. The
`hashmap_portable_callbacks` test and installed example use strict C11,
`-Wall -Wextra -Werror -pedantic-errors` under GCC/Clang, and the named API
contains no mismatched function-pointer calls.

To install:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /path/to/prefix
```

To consume independently:

```cmake
find_package(Leaderboard 3.0 CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Leaderboard::leaderboard)
```

The package installs only the static archive, public headers, and generated
CMake package/export files. No `src/internal` headers are installed. A
`tests/verify_installed_consumer.cmake` CTest check installs to a scratch
prefix, configures the independent project in `examples/external_consumer`,
then builds and runs its executable. The object-library components remain
*internal*; individually installable containers/ranking/legacy archives are
not yet offered. Header and ABI compatibility is guaranteed only for clients
recompiled with matching public headers, not binaries from pre-3.0.6.

## Validation

CI should run the existing full CTest suite, the new portable-callback and
fault-injection tests, and the installed-consumer test on both Linux GCC and
Windows/MSYS2 UCRT64. Clang's `-fsanitize=function` is specifically useful
for the portable new adapters; the legacy GNU macro tests intentionally are
not claimed to be type-safe. ASan and UBSan remain complementary checks.
