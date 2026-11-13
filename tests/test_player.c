/* Public team/player API contract; active in Release builds. */
#include "player.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do {                                                     \
    if (!(expr)) {                                                           \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr);        \
        return EXIT_FAILURE;                                                 \
    }                                                                        \
} while (0)

int main(void) {
    CHECK(player_create(NULL) == NULL);
    char empty[] = "";
    CHECK(player_create(empty) == NULL);
    CHECK(player_create_with_total(NULL, 0U) == NULL);
    CHECK(player_add_championship_points(NULL, 3U) == -EINVAL);
    player_reset_championship_points(NULL);
    player_adding_points(NULL, 1U);
    player_destroy(NULL);

    char name[] = "Brigate Ebosse";
    player_t *team = player_create_with_total(name, UINT32_C(24835));
    CHECK(team != NULL);
    CHECK(team->name == name);
    CHECK(strcmp(team->name, "Brigate Ebosse") == 0);
    CHECK(team->championship_points == 0U);
    CHECK(team->total_points_tenths == UINT32_C(24835));

    CHECK(player_add_championship_points(team, 3U) == 0);
    CHECK(player_add_championship_points(team, 1U) == 0);
    CHECK(player_add_championship_points(team, 0U) == 0);
    CHECK(team->championship_points == 4U);
    CHECK(team->total_points_tenths == UINT32_C(24835));
    /* The helper detects overflow and must leave both fields unchanged. */
    team->championship_points = UINT32_MAX - 1U;
    CHECK(player_add_championship_points(team, 2U) == -EOVERFLOW);
    CHECK(team->championship_points == UINT32_MAX - 1U);
    CHECK(team->total_points_tenths == UINT32_C(24835));
    /* The historical void helper preserves the error-free behavior. */
    player_adding_points(team, 1U);
    CHECK(team->championship_points == UINT32_MAX);
    player_adding_points(team, 1U);
    CHECK(team->championship_points == UINT32_MAX);

    player_reset_championship_points(team);
    CHECK(team->championship_points == 0U);
    CHECK(team->total_points_tenths == UINT32_C(24835));
    player_destroy(team);
    /* The caller still owns the name. */
    name[0] = 'b';
    CHECK(strcmp(name, "brigate Ebosse") == 0);

    team = player_create(name);
    CHECK(team != NULL);
    CHECK(team->name == name);
    CHECK(team->championship_points == 0U);
    CHECK(team->total_points_tenths == 0U);
    player_destroy(team);
    return EXIT_SUCCESS;
}
