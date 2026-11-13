# Failure safety and adversarial testing (milestone 3.0.8)

> **Historical scope:** This records the 3.0.8 test contract. See
> [verification](verification.md) for the current test suite.

Milestone 3.0.8 extends the isolated allocation hooks and portable callbacks
from 3.0.7 with state-aware tests. **It does not change public types, exported
functions, ranking semantics, or production data-structure algorithms.**

## Failure atomicity: what is guaranteed

- `ranking_upsert` returns `0` on success. An allocation failure returns
  `-ENOMEM`, preserves the previous membership, score and order-statistics,
  and leaves existing entry identities usable. On first insertion the entry
  may briefly exist in the AVL tree before a failed hashmap insertion;
  rollback removes that node before returning to the caller.
- An unchanged-score upsert does not allocate.
- `hashmap_base_put` preserves existing key/value associations on failure.
  A failed operation **may still increase hashmap table capacity** before a
  later key-duplication failure. The guarantee is logical state, not unchanged
  internal allocation/layout. Hashmap table rehashing must not duplicate or
  prematurely finalize existing keys.
- Insertion into BOSTree fails without changing existing nodes, order or
  retained references. Removing or destroying nodes invokes the finalizer
  exactly once for each attached node, independently of external weak refs.
  Retained node handles become tombstones and remain releasable until their
  last external reference is dropped.
- The allocation hooks only cover calls routed through `leaderboard_alloc_*`
  in hashmap, ranking and BOSTree. They do **not** intercept arbitrary libc
  allocations, user-defined callbacks, the legacy PRNG, or external consumers.

## Deterministic tests in normal CTest

| CTest name | Verification |
| --- | --- |
| `allocation_failures` | Original 3.0.7 checks for constructors, inserts and ranking updates |
| `allocation_failure_matrix` | Sweep 24 failure positions per operation; verify constructor, ranking insert/score-update, collision-heavy hashmap rehash, owned-key duplication, BOSTree insertion and tombstoned retained references |
| `api_adversarial_edges` | Invalid arguments, missing keys, null callbacks, overflow, extreme scores, missing ranks and public lifecycle edges |
| `hashmap_adversarial` | Input-driven hashmap model, intentionally colliding hashes, duplicate inserts, removal, safe iterator deletion, clear/reset and 32-key membership audit |
| `bostree_adversarial` | Input-driven AVL model, duplicate insertions, deletion, rank/select/navigation, parent/child metadata, balance and exactly-once finalization, including retained references |

Both adversarial executables generate twelve deterministic 2,048-byte
sequences using xorshift32 and one structured sequence. Input length is
bounded to 4,096 bytes per fuzz invocation, with at most 32 logical keys.
Their invariants use the private representations **only in test builds**.
The same `LLVMFuzzerTestOneInput` entry point is used for continuous fuzzing.

Run the default smoke matrix:

```sh
cmake -S . -B build-308 -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DLEADERBOARD_WARNINGS_AS_ERRORS=ON
cmake --build build-308
ctest --test-dir build-308 --output-on-failure
```

Expected CTest registration after this change: **21 tests**. In normal builds,
no fuzzing runtime is required. CTest's deterministic adversarial tests are
reproducible smoke tests, **not** coverage-guided fuzz campaigns.

## Optional coverage-guided fuzzing

With Clang and its libFuzzer runtime:

```sh
cmake -S . -B build-fuzz -G Ninja -DCMAKE_C_COMPILER=clang \
  -DCMAKE_BUILD_TYPE=Debug -DLEADERBOARD_BUILD_FUZZERS=ON
cmake --build build-fuzz --target fuzz_hashmap_libfuzzer fuzz_bostree_libfuzzer
./build-fuzz/tests/fuzz_hashmap_libfuzzer -runs=100000 -max_len=4096
./build-fuzz/tests/fuzz_bostree_libfuzzer -runs=100000 -max_len=4096
```

The opt-in `leaderboard_fuzz_core` is independently instrumented with
`-fsanitize=fuzzer-no-link,address,undefined` so instrumentation covers the
container implementations, not merely input parsers. Fuzzer executables link
libFuzzer, ASan and UBSan. Do not use this option with MSYS2/GCC; the ordinary
CTest tests remain available there.

To retain and replay findings, give each fuzzer an explicit corpus directory
and run it with the same directory in subsequent sessions. Pin the compiler
version, sanitizer configuration and corpus state in any thesis/release data.
Save reproducing inputs and minimize crashes with libFuzzer's
`-minimize_crash=1` before creating a regression test. If tests find a bug,
fix the library in a separate clearly attributed commit and preserve the
triggering input in a regression fixture.

## Limitations and release gate

This milestone sweeps a bounded number of allocation attempts for selected
operations. It cannot prove correctness for every possible allocation
failure, memory limit, malformed pointer, thread interleaving, or callback
behavior. The library remains single-threaded, relies on valid pointer
lifetimes supplied by callers, and is not intended to withstand arbitrary
memory corruption.

The exit gate is that the deterministic 21-test suite passes on Linux GCC
and Windows/MSYS2 UCRT64; the failure matrix passes under ASan/UBSan where
supported; and there are no unresolved reproducible crashes in an explicitly
recorded Clang/libFuzzer campaign. Fuzzing run counts and findings must be
reported honestly rather than interpreted as formal correctness proof.
