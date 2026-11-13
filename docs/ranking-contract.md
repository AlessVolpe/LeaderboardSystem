# Ranking abstraction contract (3.0.5)

> **Historical scope:** This covers the generic API introduced in 3.0.5,
> not the later team model or a fixture simulator. See
> [current architecture](current-architecture.md).

Milestone 3.0.5 introduces a small, domain-neutral ranking API in
[`include/ranking.h`](../include/ranking.h), implemented by
[`src/ranking.c`](../src/ranking.c). It uses the stabilized hashmap and
order-statistics AVL tree from milestones 3.0.2–3.0.4.

## Data model and ordering

- Every entry has a nonempty NUL-terminated **identifier** and a signed
  64-bit integer **score**. Identifiers are unique and compared by `strcmp`.
- Sorted order is **descending score**, then **ascending identifier**. Ties
  therefore have distinct, deterministic positions rather than shared ranks.
- Rank is **zero-based**: position 0 is the highest-ranked entry. `ranking_at`
  and `ranking_get` agree on this numbering.
- Scores are exact integers. Consumers needing fractional point values should
  choose a fixed-point scale externally; neither floating-point rounding nor
  Fantacalcio-specific rules are imposed by the library.
- IDs are byte strings without embedded NULs and comparisons are
  locale-independent; no Unicode normalization or case folding is performed.

## API

| Function | Contract |
| --- | --- |
| `ranking_new()` | Allocate an initially empty ranking; NULL on allocation failure |
| `ranking_destroy(r)` | Free all entries, keys, index and tree; NULL accepted |
| `ranking_size(r)` | Number of distinct identifiers; 0 for NULL |
| `ranking_upsert(r, id, score)` | Insert an ID, or replace its score; repeated identical scores are no-ops |
| `ranking_remove(r, id)` | Remove a participant; `-ENOENT` when missing |
| `ranking_get(r, id, &score, &rank)` | Look up score and zero-based rank; outputs may be NULL |
| `ranking_at(r, rank, &id, &score)` | Select by zero-based rank; outputs may be NULL |

Mutations return 0 on success, negative errno-style results otherwise:
`-EINVAL` for NULL arguments or empty identifiers, `-ENOENT` for missing
entries/out-of-range ranks, `-ENOMEM` on allocation failure, and `-EOVERFLOW`
if the tree cannot add another node (including temporary growth on updates).
`ranking_new()` returns NULL rather than an errno code.

For example:

```c
ranking_t *r = ranking_new();
if (r != NULL) {
    ranking_upsert(r, "alice", 75);
    ranking_upsert(r, "bob", 75);
    ranking_upsert(r, "alice", 80);
    /* alice has rank 0; bob has rank 1. */
    ranking_destroy(r);
}
```

## Data structures and ownership

The hashmap stores a borrowed key pointing to each entry's owned, immutable
identifier, and a borrowed value pointing to the entry. The AVL tree stores
separate heap-allocated ordering keys `(score, identifier)` and borrowed entry
pointers. Its finalizer frees **only** the ordering key. The ranking layer
owns the entry allocations and releases them on removal/destruction.

On first insertion, the identifier is copied, so callers may reuse or mutate
the original buffer immediately. `ranking_at` returns a borrowed identifier
which remains stable across score updates, including changes in rank. It is
invalid after removing that entry or destroying the ranking. The API exposes
neither mutable entries nor internal tree nodes.

### Atomicity of score updates

The ranking never mutates an attached BOSTree key in place: doing so would
break the binary-search invariant. Instead, it allocates a new ordering key,
**inserts the new tree node first**, and only after success removes the old
node and updates the entry's node reference and score. The old ordering key is
finalized by BOSTree. If allocation fails before insertion succeeds, the old
score and rank are unchanged. No external references to internal nodes are
exposed, and no temporary change is observable in this single-threaded API.

New insertions similarly roll back their temporary tree node if registration
in the hashmap fails. Removal deregisters the ID, removes the associated
BOSTree node, then frees the entry. Destruction finalizes tree keys first and
releases the hashmap entries afterwards.

The ranking does not provide concurrency, persistence, iterator stability,
custom comparators, or custom allocators. It also does not guarantee stable
rank positions across unrelated updates; it guarantees stable ID identity.

## Complexity

Let `n` be the number of entries. The hashmap uses open addressing and has
expected O(1) lookup, with worst-case O(n) for adverse collisions. The AVL
tree maintains O(log n) height.

| Operation | Expected time | Worst case |
| --- | --- | --- |
| `ranking_upsert`, new entry | O(log n) amortized | O(n) during hashmap rehash/collision scenarios |
| `ranking_upsert`, score changed | O(log n) | O(n) for a hashmap lookup with severe clustering |
| `ranking_upsert`, unchanged score | O(1) | O(n) |
| `ranking_remove` | O(log n) expected | O(n) for hashmap lookup/removal under clustering |
| `ranking_get` with rank | O(log n) | O(n) for hashmap lookup under clustering |
| `ranking_at` | O(log n) | O(log n) |
| `ranking_destroy` | O(n) | O(n) |

The ranking uses O(n) persistent space, with a temporary additional AVL node
and ordering key during score updates. `strcmp` and hashing costs depend on
identifier length; the bounds above treat IDs as bounded-length strings.

## Verification

`ranking_regressions` covers invalid/empty inputs, exact score ordering and
lexical ties, score updates crossing the entire ranking, `INT64_MIN` and
`INT64_MAX`, no-op updates, removal/reinsertion, identifier copy/lifetime,
and hashmap expansion with 1,024 additional entries.

`ranking_differential` executes 15,000 deterministic upsert, remove, and lookup
operations, with xorshift32 seed `0x6D305A17`, over 193 independently modeled
participants. After periodic mutations it independently sorts a plain array
with `qsort` and compares *every* `ranking_at` result and `ranking_get` rank,
score and presence against that model, then drains the ranking.

Both tests use `CHECK` macros that remain enabled in Release builds. Build
with GCC/Clang, `-Wall -Wextra -Werror`, and use ASan/UBSan when available.
These tests evaluate functional correctness; they are not benchmarks or a
proof that arbitrary memory allocation cannot fail.

## Deferred

The legacy name-ordered example CLI remains unchanged. Future milestones may
integrate the ranking API into an application, introduce ranking snapshots or
opaque entry handles, and add controlled performance benchmarks. Fantacalcio
score computation, Monte Carlo simulation, and Python bindings are **not**
implemented as part of 3.0.5.
