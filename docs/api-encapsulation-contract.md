# API boundaries and module contract (3.0.6)

Milestone 3.0.6 hardens the native C interface without rewriting the AVL
or hashmap algorithms. The ranking API remains domain-independent, and the
existing `Leaderboard::leaderboard` archive and public include paths remain
available.

## Internal compilation modules

The CMake build compiles three *object-library* components independently:

| Internal target | Sources | Dependencies |
| --- | --- | --- |
| `leaderboard_containers_obj` | `hashmap.c`, `bostree.c` | Standard C, private BOSTree representation |
| `leaderboard_ranking_obj` | `ranking.c` | Public hashmap base and BOSTree APIs |
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

## Hashmap boundary (still deferred)

The typed hashmap macro embeds `hashmap_base`, so the hashmap representation
cannot simply be made opaque without redesigning its storage and macro API.
Direct access to hashmap bookkeeping remains possible in 3.0.6. Rather than
introducing an untested breaking migration, opaque hashmap handles and removal
of GNU statement-expression macros are deferred to later milestones.

## Verification and release scope

`bostree_public_api` compiles with the **public** header only and checks
opaque pointers, the legacy type aliases, borrowed accessors, rank/select,
invalidated retained nodes and finalizer-time payload access. Existing AVL
invariant and deletion tests compile with the explicitly private header,
retaining structural assertions without exposing representation to consumers.
Ranking's existing regression and differential tests must remain green.

No changes to algorithmic complexity are intended in this milestone. The
new accessors take O(1) time. Allocation-failure injection, an installable
modular CMake package, stronger hashmap encapsulation, and a comprehensive
ABI compatibility policy remain deferred.
