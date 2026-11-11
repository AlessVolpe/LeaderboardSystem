# BOSTree stabilization: contract and verification (3.0.3)

## Structure and invariants

BOSTree is an **AVL-balanced order-statistics binary search tree**. The
comparator defines a strict ordering on keys; comparator equality denotes **the same key**. The implementation stores
the following metadata on each node:

- `left_child_count` is the exact number of nodes in the left subtree.
- `right_child_count` is the exact number of nodes in the right subtree.
- `depth` is the **height**, with a leaf at depth 0 and an empty subtree at -1.
- Parent links are reciprocal: a child's `parent_node` points to its parent,
  and the root's parent is `NULL`.
- Every key in the left subtree compares less than its ancestor's key; every
  key in the right subtree compares greater.
- The difference between left and right subtree heights is at most 1.

The node count is `left_child_count + right_child_count + 1` for any node, and
zero for an empty tree. The implementation retains separate child counts in the
existing public structure for compatibility; rotations and insertions **recompute** metadata bottom-up instead of
modifying counts speculatively.
The maximum representable tree size is `UINT_MAX` nodes.

## API and ownership

- `bostree_new` requires a non-NULL comparator and returns `NULL` on invalid
  input or allocation failure.
- `bostree_insert` borrows the supplied key/data pointers. It returns the
  inserted node, or **the existing node** if the comparator reports equality.
  In the latter case the original key and data pointers are unchanged and the
  caller retains ownership of the rejected arguments. `NULL` denotes invalid
  input, allocation failure or the size limit.
- The library does **not** copy or free keys or values automatically. By
  default both are borrowed. An optional node finalizer (`free_func`) is
  invoked during node disposal and may release payloads if the application
  chose to transfer ownership. This callback owns the payload policy.
- `bostree_lookup` returns `NULL` for an absent key. `bostree_select(tree, i)`
  returns the node at **zero-based** rank `i`, or `NULL` if out of range.
  `bostree_rank(node)` is zero-based and requires a valid, non-NULL node from a
  live tree. Therefore `bostree_rank(bostree_select(tree, i)) == i` holds for
  every valid index.
- `bostree_next_node` and `bostree_prev_node` return `NULL` at the ends of
  the traversal and for a NULL input. They do not mutate the tree.
- A weak reference may be temporarily acquired and released **while the tree
  remains alive**. Existing weak-reference lifetime and invalidation semantics
  are not fully specified; all external references **must be released before**
  `bostree_destroy`. This remains future work alongside deletion.
- `bostree_remove` is **not implemented** in this milestone; calling it
  terminates the process. Deletion and post-deletion rebalancing belong to 3.0.4.

## Complexity

| Operation               | Worst-case time after successful insertion |
|-------------------------|--------------------------------------------|
| Insertion / lookup      | O(log n)                                   |
| Rank / select           | O(log n)                                   |
| Successor / predecessor | O(log n)                                   |
| Full destruction        | O(n)                                       |

The AVL height bound is maintained by single and double rotations. This is a *structural* guarantee and does not assert
anything about the business-domain
ranking semantics. Ranking by score requires a comparator over scores and a
stable tie-breaking policy, which will be addressed in the typed ranking layer.

## Verification

`bostree_invariants` validates the BST ordering, parent links, per-child
subtree counts, heights, AVL balance, node cardinality, and consistency of
lookup/rank/select/navigation. It covers all four elementary insertion
rotations (LL, RR, LR, RL), sorted ascending and descending inputs (2,048
entries each), and a Fisher-Yates shuffle of 4,096 entries with xorshift32
seed `0xC0FFEE21`. Duplicate-key semantics and finalizer invocation counts
are checked explicitly. Unlike `assert`, the regression checks remain active
in Release builds.

Run the full test suite with CTest, and additionally use
`-DLEADERBOARD_ENABLE_SANITIZERS=ON` on supported GCC/Clang toolchains. For
thesis evaluation, compare the observed tree height and operation cost across
random, sorted and adversarial sequences, and record compiler, CPU, seed,
input distributions and optimization flags.

## Deferred work

Node deletion, weak-reference lifetime redesign, transactional payload
ownership, a typed ranking API, and score-based ordering are not part of 3.0.3.
The legacy example CLI still uses an unsafe ownership model; it must not be
used as evidence of engine memory safety.
