# Current architecture and integration boundaries

**Baseline:** post-player-refactor `master`, package version **3.0.10.5**.
This is the native C library, not a working Fantacalcio simulation engine.

## Native components

CMake combines the internal `leaderboard_containers_obj`,
`leaderboard_ranking_obj` and `leaderboard_legacy_obj` targets into a single
static `leaderboard` library exported as `Leaderboard::leaderboard`.
These object targets are not independently installable libraries.

| Layer | Interface | Role |
| --- | --- | --- |
| Hashmap | `hashmap_base.h`, `hashmap_typed.h` | Keyed lookup and portable typed callbacks |
| AVL order-statistics tree | `bostree.h` | Search, deletion, zero-based rank and selection |
| Generic ranking | `ranking.h` | Opaque `ranking_t`, descending signed 64-bit score and identifier tie-break |
| Team record | `player.h` | Borrowed team name, championship points, fantasy total in tenths |
| PRNG / example utilities | `mtwister.h`, legacy helpers | MT19937 and demonstration support |

New code should prefer the portable C11 typed hashmap API; deprecated
`HASHMAP(...)` macros require GNU extensions. Internal representations are
private and are included by designated white-box tests only.

## Team records versus generic rankings

A `player_t` represents a Fantacalcio **team**, despite its historical name.
It borrows its NUL-terminated name and does not treat it as a unique key.
`total_points_tenths` stores the nonnegative, one-decimal season score
(e.g. `24835` is `2483.5`) independently of fixture permutations.
`championship_points` accumulates simulation-dependent standings points and
can be reset without changing the total. Addition is checked for overflow.

The independent `ranking_t` copies its identifiers and uses a **single**
signed 64-bit score. Its deterministic order is descending score, then
ascending ID. It does not infer championship rules, fantasy totals, or
season ranking tie-break policies. The interactive CLI's name-ordered tree
and the batch ranking example demonstrate APIs, not fixture simulation.

The change from `player->score` to the two `uint32_t` fields changes the
public structure layout. Native consumers of the old layout must be rebuilt.
Destroying a player does not free its borrowed name; an identifier borrowed
from `ranking_t` becomes invalid after removal/destruction.

## Future integration, not implemented

Teams will be registered and stored by the Python web application/database.
The initial basic Fantagazzetta fixture list will be supplied externally;
a future C engine will generate calendar permutations, simulate standings,
and return results to the application. Calendar import, permutations,
tie-break policies, persistence and Python bindings remain separate design
and implementation tasks. None are implied by the generic ranking API.

See the [team/player contract](team-player-model.md) for field-level details.
