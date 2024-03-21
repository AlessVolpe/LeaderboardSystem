//
// Created by aless on 21/03/2024.
//

#include <stdlib.h>

#include "player.h"

player_t* player_create(char* name) {
    player_t* player = calloc(1, sizeof(player_t));
    player->name = name;
    player->score = 0;
    return player;
}

void player_assing_points(player_t* player, uint32_t points) {
    player->score += points;
}
