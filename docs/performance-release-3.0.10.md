# Milestone 3.0.10: Performance and release readiness

> **Historical scope:** This records the 3.0.10 benchmark milestone. The
> current CMake project/package version is 3.0.10.5.

## Scope

The benchmark executable uses **public APIs only** and measures three independently
constructed data structures: `hashmap_base_t`, `bostree_t`, and `ranking_t`.
Operations are ordered by a fixed xorshift32 shuffle; fixture identifiers are
unique, and the seed changes deterministically per round. The benchmark
validates results while measuring operations and emits machine-readable CSV.
Checksums keep the results observable. **No timing-based test assertion** is
used, so CI is not sensitive to machine contention.

### Build

```sh
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DLEADERBOARD_BUILD_BENCHMARKS=ON -DLEADERBOARD_WARNINGS_AS_ERRORS=ON
cmake --build build-release
ctest --test-dir build-release --output-on-failure
./build-release/leaderboard_bench 1000 5 > benchmark-1000.csv
./build-release/leaderboard_bench 10000 5 > benchmark-10000.csv
./build-release/leaderboard_bench 100000 3 > benchmark-100000.csv
```

On Windows/MSYS2, invoke `./build-release/leaderboard_bench.exe` instead.
The benchmark accepts `[entries] [rounds]`, capped at 100,000 entries and
30 rounds; defaults are 10,000 and 3. Inputs outside the supported range fail.

### Interpreting the CSV

Columns: `structure,operation,count,iteration,cpu_seconds,ns_per_op,checksum`.
The reported timing is **`clock()` CPU time**, not wall-clock latency. Timed
regions include public API calls and their correctness checks, plus some
small loop overhead. Allocation is included in insertion and update timings.
Fixture setup is outside the timed regions. `ns_per_op` means CPU nanoseconds
per operation, **not** an individual operation latency measurement. On short
workloads the clock resolution causes large quantization errors. Use the
larger workloads for comparisons. `clock()` measures process CPU time, so
results can differ with CPU model, compiler, configuration, allocator, OS,
and concurrent workloads. Do not compare Debug/ASan timings to Release.

The benchmark contains no independently standardized baseline, no stable
cross-machine performance budget, and no automatic regression threshold.
Do not use its absolute timing numbers as contractual guarantees.

### Complexity expectations

- Hashmap lookup, insertion and removal: expected average constant-time,
  with adversarial collisions and resizing influencing worst-case behavior.
- BOSTree insertion, lookup and deletion: logarithmic in tree size; `rank`
  and `select` use maintained subtree counts.
- Ranking insertion, update and deletion: combines hashmap operations with
  balanced-tree operations; `ranking_at` is logarithmic.

The benchmark is a practical smoke/characterization workload rather than a
formal proof of asymptotic bounds. It currently measures unique keys, uniform
pseudo-random operation ordering, and mixed ranking scores with ties. It does
not measure pathological custom hash functions, concurrency, multithreaded
contention, very long identifiers, or heap-peak statistics.

For optional Linux peak resident memory observations, use:

```sh
/usr/bin/time -v ./build-release/leaderboard_bench 100000 3 > benchmark.csv
```

The peak applies to the **whole process** (fixtures and benchmark phases),
not to an individual container's heap allocation. Keep platform, compiler,
CPU, release flags, and input sizes with any published report.

## Release verification checklist

1. Verify the declared CMake project version against the package being
   tested, and check the independent consumer's minimum required version.
2. Run the full default CTest suite with GCC and Clang on Linux, and GCC UCRT64
   on Windows. Run ASan/UBSan on Linux; the sanitizer option verifies runtime
   linking during CMake configure.
3. Configure a separate Release build with `LEADERBOARD_BUILD_BENCHMARKS=ON`,
   run CTest, and capture CSV for multiple workload sizes.
4. Install into a temporary prefix and build the standalone
   `examples/external_consumer` project using its declared `find_package`
   requirement and `Leaderboard::leaderboard`. A compatible minimum version
   does not, by itself, guarantee ABI compatibility with old `player_t` layouts.
5. Verify strict C11 public-header checks, public API error contracts, and
   the optional libFuzzer workflow remain green.
6. Review LICENSE, THIRD_PARTY_NOTICES.md, and package contents before tagging.

Versioning uses the project's agreed sequence through 3.0.12, then 3.1.0;
compatibility requirements take precedence if public ABI/API breaks.
