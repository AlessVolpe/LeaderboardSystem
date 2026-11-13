# Verification guide

The CTest suite now includes invariants, regressions, differential workloads,
failure injection, PRNG vectors, CLI scenarios, installation checks, and the
team/player model. Tests provide evidence for the checked cases, not proof
of total correctness or performance guarantees.

## Debug build and tests

Use CMake 3.16+, Ninja, and GCC or Clang:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DLEADERBOARD_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest --test-dir build -N` lists configured tests. The new team cases are
`player_model_contract` and `leaderboard_cli_team_names`; ranking and
container invariants, deletion lifecycle, failure injection, and
`mtwister_reference_vectors` run in the ordinary suite as well.

## Supported CI targets

The GitHub workflow runs Linux/GCC, Windows/GCC in the **MSYS2 UCRT64**
environment, Linux ASan/UBSan, and separate Linux Clang/libFuzzer campaigns
for hashmap and BOSTree. Do not reuse a CMake build directory configured
for a different generator. The Windows CI path is Ninja, not NMake.

For optional sanitizer testing where the runtime is available:

```sh
cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DLEADERBOARD_ENABLE_SANITIZERS=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

CMake checks that sanitizer instrumentation compiles **and links**.

## Installed package

The `installed_consumer` CTest validates an isolated installation and
public-header-only client. A manual external consumer check follows:

```sh
cmake --install build --prefix "$PWD/installed"
cmake -S examples/external_consumer -B build-consumer -G Ninja \
  -DCMAKE_PREFIX_PATH="$PWD/installed"
cmake --build build-consumer
ctest --test-dir build-consumer --output-on-failure
```

Current CMake version: `3.0.10.5`. The external consumer still requests
`Leaderboard 3.0.10`, accepted under the package's SameMajorVersion policy.
This version check does not guarantee ABI compatibility with clients
compiled against the previous `player_t` layout.

## Optional benchmark

Set `LEADERBOARD_BUILD_BENCHMARKS=ON` in a separate Release build, run CTest,
then execute `leaderboard_bench` with entry and round counts. The CSV reports
process CPU time from `clock()`. Timing is not a CI acceptance threshold;
record toolchain, hardware, build options, seed and workloads with reports.
See [performance and release](performance-release-3.0.10.md) for details.
