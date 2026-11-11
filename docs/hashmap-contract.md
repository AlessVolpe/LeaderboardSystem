# Hashmap stabilization: contract and verification (3.0.2)

## Representation and invariants

The implementation uses **open addressing with linear probing** and
**backward-shift deletion**. The capacity is a power of two and at least 32 once
allocated. The table never exceeds a load factor of **0.75**, including after
a successful insertion. The entry count (`size`) is exactly the number of
non-null keys. Every inserted key has a non-null value.

Lookup stops at the first empty entry; thus removing an element must not create
a hole that disconnects later entries from their initial hash slot. Deletion
moves subsequent entries backward when required to preserve this invariant.
Rehash relocates entries without duplicating or freeing keys or values.

## Ownership

- By default, the hashmap **borrows keys and values**. Their lifetime must
  cover their residence in the map.
- Install a *pair* of `key_dup`/`key_free` callbacks on an empty map to make the
  hashmap own key copies. An incomplete pair returns `-EINVAL`; trying to
  change callbacks with entries present returns `-EBUSY`.
- The hashmap **never owns values**. The caller remains responsible for all
  values, including those returned by `remove`.
- `remove` frees an owned key exactly once; `clear`, `reset` and `cleanup` free
  any remaining owned keys exactly once.
- A failed duplicate insert returns `-EEXIST` without changing the entry or
  invoking `key_dup`. If `key_dup` fails, `put` returns `-ENOMEM` without
  inserting the entry. Table *capacity* may still have grown during a failed
  insertion; existing associations remain valid.

## Error and iteration semantics

- `put` returns `0`, `-EEXIST`, `-EINVAL`, `-ENOMEM`, `-ENOSPC`, or `-EOVERFLOW`.
- `reserve` returns `0`, `-ENOMEM`, `-EINVAL`, or `-EOVERFLOW`. It can adjust
  the capacity to which `reset` will shrink. It never drops existing entries.
- `get` and `remove` return `NULL` for absent keys. Null values cannot be
  inserted, so absence is unambiguous.
- A normal `put`, `remove`, `reset`, `clear`, or `reserve` can invalidate
  iterators. Use `hashmap_base_iter_remove` to remove *the current entry* while
  iterating, or use the typed `hashmap_foreach_*_safe` macros.
- `reset` clears the map and *attempts* to shrink to `table_size_init`. If the
  shrink allocation fails, the empty larger table is retained. `clear` retains
  the allocated capacity. `cleanup` releases the table and zeroes the object.

## Asymptotic costs

| Operation | Expected | Worst case |
| --- | --- | --- |
| Insert / lookup / remove | O(1) with a well-distributed hash | O(n) |
| Rehash | O(n) | O(n²) with adversarially colliding keys |
| Iteration / clear / reset | O(capacity) | O(capacity) |

A maximum load factor is not a proof of constant-time behavior under
adversarial collisions. This is a non-cryptographic, non-concurrent hash map.

## Verification strategy

`hashmap_regressions` checks owned and borrowed keys, callback misuse,
allocation failure, duplicate insertion, capacity boundaries, wrap-around
collision chains, backward-shift removal, safe iterator deletion, and
14,000 deterministic randomized operations against a simple reference model.
`hashmap_typed_macros` exercises the GNU C type-safe wrappers.

The tests use checks that remain active in Release builds; the randomized
seed is fixed so failures are reproducible. When collecting performance
results for a thesis, record compiler, optimization level, CPU, workload
size, collision profile and random seed. Benchmark against at least one
independent implementation and include confidence intervals and variance.
