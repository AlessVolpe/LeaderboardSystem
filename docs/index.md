# LeaderboardSystem documentation

The current CMake package version is **3.0.10.5**. This index separates the
implemented interfaces from historical milestone descriptions.

## Current reference

- [Current architecture](current-architecture.md): native modules, public
  interfaces, ownership and future Python/fixture integration boundaries.
- [Verification](verification.md): CMake, CTest, CI, installed consumer,
  sanitizers and optional benchmarks.
- [Team/player model](team-player-model.md): championship points, fantasy
  totals in tenths, borrowed names and compatibility.
- [Contributing](../CONTRIBUTING.md): toolchains and development practices.

## Historical milestone contracts

- [3.0.2 Hashmap stabilization](hashmap-contract.md)
- [3.0.3 BOSTree invariants](bostree-contract.md)
- [3.0.4 BOSTree deletion and lifecycle](bostree-deletion-contract.md)
- [3.0.5 Generic ranking](ranking-contract.md)
- [3.0.6 API encapsulation](api-encapsulation-contract.md)
- [3.0.7 Portable callbacks and packaging](portable-callbacks-failures-packaging.md)
- [3.0.8 Adversarial testing](failure-adversarial-contract.md)
- [3.0.9 MT19937 and integration](portability-prng-3.0.9.md)
- [3.0.10 Benchmark and release readiness](performance-release-3.0.10.md)

Deferred work and compatibility remarks in these documents are relative to
the named milestone, not necessarily to the current codebase. Consult the
current public headers, architecture guide and tests for present behavior.
