# Team/player model: simulation-independent totals and league standings

## Scope

`player_t` is the small, public C representation of a Fantacalcio **team**.
Its `name` is a human-readable, NUL-terminated team name; it is not a unique
machine identifier. It can contain spaces. The legacy CLI still uses UUIDs as
pool keys and a **name-ordered** BOSTree for demonstration purposes. The CLI is
not the league simulator and should not be interpreted as the competitive
ranking algorithm.

## Data contract

| Field | Type | Unit | Depends on fixture permutation? |
| --- | --- | --- | --- |
| `name` | `char *` | Borrowed team-name string | No |
| `championship_points` | `uint32_t` | Standings points (3 win / 1 draw / 0 loss) | Yes |
| `total_points_tenths` | `uint32_t` | Aggregate fantasy score multiplied by 10 | No |

A season total of **2483.5** is stored as `24835`. Use integer division and
remainder to format `total_points_tenths / 10` and `% 10` without floating
point. The scale supports scores with at most one decimal digit. All totals
are nonnegative. Input conversion from Python should validate nonnegativity,
at most one decimal digit, and the `uint32_t` bound *before* passing it into C;
never truncate a floating-point value silently.

`player_create_with_total(name, total)` starts each calendar simulation with
zero championship points and an unchanged season aggregate. Call
`player_add_championship_points(player, delta)` after a simulated result; the
function checks overflow (`-EOVERFLOW`) and rejects a NULL player (`-EINVAL`).
Match-rule validation belongs to the simulation layer; it may pass `3`, `1`,
or `0`, but the arithmetic helper is intentionally general. Before the next
permutation, `player_reset_championship_points(player)` resets **only** the
simulation-dependent points.

`player_create(name)` remains available with both totals initialized to zero;
`player_adding_points` is a compatibility shim. Its historical `void` return
cannot report overflow; prefer the checked helper.

### Ownership

Constructors allocate only the `player_t` object. They **borrow** `name` and
do not copy or free it. The caller must keep the string alive while the player
is used, then release it separately if heap-allocated. `player_destroy` frees
only the structure and accepts NULL. The legacy CLI owns its input names in
a separate list and frees them after destroying the tree's players.

### Boundaries with ranking_t

`ranking_t` deliberately remains a generic independent API with a single
signed 64-bit score, ordered descending with identifier-based tie-breaking.
This patch does not reinterpret that score as championship or fantasy points,
and it does not introduce any implicit packing or league tie-break rule.
Fixture scheduling and tie-break policy will be specified in the future C
simulation engine, which Python can invoke to collect and analyze results.

### Compatibility

Replacing the historical `score` member **changes the public struct layout
and breaks sources that access `player->score`**, even though the old constructor
and `player_adding_points` symbols remain available. It is also an ABI change:
recompile/relink clients and assign a new library version before distributing
it as a release. This standalone post-3.0.10 patch intentionally does not
claim to be a numbered release or alter the CMake project version.
