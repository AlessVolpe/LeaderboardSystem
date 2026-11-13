# API boundaries and module contract (3.0.6)

Milestone 3.0.6 hardens the native C interface without rewriting the AVL
or hashmap algorithms. The ranking API remains domain-independent, and the
existing `Leaderboard::leaderboard` archive and public include paths remain
available.

## Internal compilation modules

The CMake build compiles three *object-library* components independently:

| Internal target | Sources | Dependencies |
| --- | --- | --- |
| `leaderboard_containers_obj` | `hashmap.c`, `bostree.c` | Standard C, private hashmap and BOSTree representations |
| `leaderboard_ranking_obj` | `ranking.c` | Public opaque hashmap base and BOSTree APIs |
| `leaderboard_legacy_obj` | `mtwister.c`, `player.c`, `commons.c` | Legacy utilities only |

CMake combines these object files into the unchanged `leaderboard` **static
archive**, avoiding duplicate definitions and preserving the existing archive output
and `Leaderboard::leaderboard` link target. These object
targets are **internal build units, not independently installed libraries**.
Installing separately linkable module targets remains future work.

The ranking component's dependency direction is one-way: ranking depends on
the containers, not the other way around. The legacy utility component is not
required by ranking or either container. No Fantacalcio scoring domain logic
is introduced.

## Public and private BOSTree interfaces

`include/bostree.h` now forward-declares `bostree_t` and `bostree_node_t`.
It deliberately does **not** expose node or tree fields. The private
representations live in `src/internal/bostree_internal.h` and are compiled
only by the BOSTree implementation and explicitly white-box invariant tests.

Borrowed accessors: `bostree_node_key`, `bostree_node_data`, and
`bostree_node_is_valid`. The key accessor returns `const void *` to discourage changing AVL ordering
keys in-place. Data remains mutable by the owning application. Both pointers
are borrowed, not copies, and cannot be used after finalization.
The BOSTree callback's existing disposal sequence is unchanged: the node is
marked invalid *before* calling the finalizer, but the key/data pointers
remain accessible **during** the callback. After finalization, a retained
node has NULL payload fields and must be released according to the 3.0.4
reference lifecycle contract.

The new names satisfy the project's `_t` typedef policy. Existing `BOSTree`,
`BOSNode`, `BOSTree_cmp_func`, and `BOSTree_free_func` remain usable as pointer
aliases. The exported function names and C linkage remain unchanged. This is
**not full source compatibility** for clients that accessed tree or node
fields directly: those operations now fail to compile, intentionally. There
is no supported public accessor for internal AVL counts or parent pointers;
white-box tests use the private header instead.

The raw binary layout and symbol names of the underlying tree operations are
not otherwise changed, but this version does not promise a stable ABI. Only
newly compiled clients against the new public header are within the tested
compatibility boundary.

## Hashmap representation and typed API

`include/hashmap_base.h` now forward-declares `hashmap_base_t` and
`hashmap_entry_t`. The actual hash table, open-addressing entry storage,
collision metadata and callback pointers are defined solely in
`src/internal/hashmap_internal.h`. Public consumers cannot allocate a
`hashmap_base_t` on the stack or access any of these fields.

`hashmap_base_new`/`hashmap_base_destroy` manage heap-owned handles;
`hashmap_base_size`, `hashmap_base_capacity`, and
`hashmap_base_initial_capacity` expose read-only inspection. Both original
`hashmap_base` and `hashmap_entry` remain as **incomplete-type aliases**, not
as publicly defined structs. The old stack-based `hashmap_base_init` and
`hashmap_base_cleanup` are internal helpers and no longer public functions.

`HASHMAP(key_type, data_type)` still supports the original typed operations
and foreach iteration macros, but now stores a heap-owned `map_base` handle.
`hashmap_init` returns 0 or `-ENOMEM`, and callers must check that result;
`hashmap_cleanup` releases the handle and resets the pointer to NULL.
The `map_base` member remains exposed *in the typed macro's wrapper* for
compatibility, but it no longer exposes the underlying table representation.
GNU `typeof` and statement expressions are retained for the existing typed
macro API; eliminating them is future work. In particular, the legacy macros
cast type-specific hash/comparator function pointers to `const void *`
callback signatures. These incompatible function-pointer calls are not
strictly portable C: Clang's `-fsanitize=function` detects them for the
string/`strcmp` callbacks used by the existing CLI and typed tests. This
predates the opaque-handle refactor. A redesigned, type-correct callback
adapter interface is deferred; direct users of `hashmap_base_new` must supply
functions with the exact generic signatures to avoid this issue.

**Compatibility:** low-level clients that used `hashmap_base` as a complete
stack type, embedded it, or accessed table slots must migrate to the heap
lifecycle and accessor functions. This is an intentional source/ABI break,
not a claim of 2.x binary compatibility. Iterator entry pointers remain
borrowed and invalidated by rehash, clear, reset or deletion. As before,
only the safe foreach variant may remove its current entry. Hashing,
comparison, key-ownership, and finalization semantics are unchanged.

The `hashmap_public_api` test includes only public headers and validates
opaque handles, insertion/duplicate behavior, iterator access, key-copy
ownership and typed wrapper operations. Existing `hashmap_regressions`
compile with the explicit private header to retain white-box assertions.

## Verification and release scope

`bostree_public_api` compiles with the **public** header only and checks
opaque pointers, the legacy type aliases, borrowed accessors, rank/select,
invalidated retained nodes and finalizer-time payload access. Existing AVL
invariant and deletion tests compile with the explicitly private header,
retaining structural assertions without exposing representation to consumers.
Ranking's existing regression and differential tests must remain green.

No changes to algorithmic complexity are intended in this milestone. The
new accessors take O(1) time. Allocation-failure injection, an installable
modular CMake package, removal of GNU hashmap macro extensions, and a comprehensive
ABI compatibility policy remain deferred.
