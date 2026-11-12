# BOSTree deletion and lifecycle contract (3.0.4)

This document extends the insertion and ordering contract in
[`bostree-contract.md`](bostree-contract.md). It describes the behavior after
milestone 3.0.4; the older milestone's deferred-work notes remain historical.

## Deletion algorithm

`bostree_remove(tree, node)` removes an **attached node** in `O(log n)` time.
It does not compare or copy key/value payloads during deletion:

1. With zero or one child, splice the child into the removed node's position.
2. With two children, find the in-order successor and **physically transplant
   its node** into the removed node's position. Both the direct-right-child
   and deep-left-descendant cases are handled.
3. Recompute child counts and heights from the affected position up to the
   root, applying single or double AVL rotations as necessary.
4. Dispose of the *original* removed node and release its tree-owned reference.

Physical transplantation preserves the identity and payload of surviving
nodes, which matters when callers hold retained node pointers. It also prevents
accidental finalization of the successor's payload.

After every successful deletion, the tree still satisfies the strict BST
ordering, reciprocal parent links, correct `left_child_count` and
`right_child_count`, height metadata, and AVL balance factors in `[-1, 1]`.
`rank`, `select`, successor and predecessor remain zero-based and consistent.

A `NULL` tree/node, a retained but already invalidated node, or an attached
node from a different live tree is ignored. **A dangling pointer cannot be
validated safely**; callers must never pass a node after its final reference
has been released. The public deletion API remains `void` for compatibility.

## Ownership and reference lifecycle

Each attached node starts with a reference count of one, owned by the tree.
`bostree_node_weak_ref(node)` adds a caller-owned reference if the node is
still valid and the 8-bit counter has not reached `UCHAR_MAX`. It returns
`NULL` for `NULL`, invalidated, or saturated nodes. It does not copy payloads.
Despite the historical `weak_*` naming, these are reference-counted handles:
they prolong the **node allocation**, but not payload lifetime after deletion.

On removal or `bostree_destroy`:

- The node is marked invalid (`weak_ref_node_valid = 0`) and detached.
- The optional `free_func` is called **exactly once**, while the tree still
  exists and with `node->key` and `node->data` available to the callback.
  The callback may free owned payloads but must not reenter or mutate the tree.
- Key, data, child links, parent link, subtree counts and height are cleared.
- The tree drops its reference. The node allocation is freed immediately
  when no external references remain; otherwise an invalid *tombstone*
  remains until the last external `weak_unref`.

A tombstone may be inspected for `weak_ref_node_valid == 0` and released, but
not used as a key/value or navigated through `next`, `prev`, or `rank`. The
latter return `NULL`, `NULL`, and `UINT_MAX` respectively on invalidated nodes.
`bostree_node_weak_unref(tree, node)` releases **one previously acquired
external reference**. Its `tree` argument is ignored for compatibility; use
`bostree_node_weak_unref(NULL, node)` after the tree has been destroyed. A
live node's tree-owned final reference is protected from external release.
The returned pointer is non-NULL only if the node remains valid.

**Reference-safety limitations:** This is a single-threaded, non-concurrent
API. The counter is 8-bit; callers must balance every successful acquisition
with one release. They must not dereference or release a node after their last
reference expires, nor access a tombstone's former payload. The public node
layout and weak-reference API are retained for compatibility; opaque handles
and stronger lifetime typing are deferred.

The tree does not implicitly own payloads. With `free_func == NULL`, payloads
remain caller-owned. With a finalizer, the application's callback determines
what gets freed and must not release the `BOSNode` object itself. Inserting a
duplicate key leaves the original node and payload unchanged.

## Complexity

| Operation | Worst-case time | Extra space |
| --- | --- | --- |
| Remove by attached node pointer | `O(log n)` | `O(1)` |
| Insert / lookup / rank / select | `O(log n)` | `O(1)` |
| Retain / release node reference | `O(1)` | `O(1)` |
| Destroy entire tree | `O(n)` | `O(1)` |

Payload callback costs are application-dependent and excluded from these
bounds. External retained tombstones require memory proportional to the
number of invalid nodes whose external reference counts remain nonzero.

## Verification

`bostree_deletion` performs all 5,040 deletion orders of a seven-node tree,
15,000 deterministic insert/remove operations against an independent presence
model (xorshift32 seed `0xA731A904`), and special-case tests for successor
transplantation, invalidation, reference saturation, callback counts,
cross-tree deletion rejection, and release after tree destruction. After
mutations, it checks BST ordering, exact subtree counts, heights, AVL balance,
parent pointers, node identities and all rank/select/navigation relations.

Checks remain active in Debug and Release builds. Run with GCC and Clang,
`-Wall -Wextra -Werror`, and use AddressSanitizer and UndefinedBehaviorSanitizer
on supported platforms. The full repository and Windows/MSYS2 CI should also
be run before merging. For a thesis, retain test seeds, toolchain versions,
and machine specifications alongside performance results.

## Deferred

The legacy CLI's ownership and casts are still unsafe and should not be used
as a memory-safety demonstration. The next milestones will address an opaque
API and typed ranking integration; neither is implemented here.
