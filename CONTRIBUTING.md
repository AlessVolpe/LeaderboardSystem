# Development and repository tracking

This repository is in the **3.0 Native Engine Foundation** stage. The C library (`leaderboard`)
and the example CLI (`leaderboard_cli`) are intentionally separate build targets. The initial
CTest tests are smoke tests, not evidence that the legacy hashmap or AVL/order-statistics
algorithms are correct.

## GCC / Clang builds

The public hashmap macros currently rely on GNU C extensions (`typeof` and statement
expressions). Use GCC or Clang; MSVC is **not** a supported compiler yet.

On Linux, install a C compiler, CMake (3.16+), and Ninja, then run:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### Windows: MSYS2 UCRT64

Open the **MSYS2 UCRT64** shell and install:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

From the repository root, run the same CMake/CTest commands above. If the build directory
was created with another generator or toolchain, delete **only** `build/` before reconfiguring.

Use a separate build directory when changing generators or compilers, for example
`build/debug-gcc` versus `build/release-gcc`.

### Optional diagnostic builds

On a GCC/Clang toolchain where the sanitizer runtime is supported, use a *separate* build tree:

```sh
cmake -S . -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DLEADERBOARD_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

Sanitizers are optional and may need additional runtime support on MSYS2. Do not assume the
legacy interactive CLI is safe to run: it still has known ownership and pointer bugs.

## What belongs in Git

Track source (`src/`, `include/`, `examples/`), test sources (`tests/`), CMake files,
documentation, and `.github/workflows/` CI configuration. These are necessary to reproduce
a build. Do **not** commit `build/`, `cmake-build-*/`, compiler output, CMake caches,
per-machine `.idea/` or `.vscode/` configuration, or test logs.

The root `.gitignore` stops *new* files from being tracked; existing tracked generated files
must be removed from the index with `git rm -r --cached`, without deleting local copies.

Commit 3.0.1 tracking fixes separately from algorithm changes. Hashmap correctness and
ownership changes belong to milestone 3.0.2, with regression tests.
