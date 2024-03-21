//
// Created by aless on 21/03/2024.
//

#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#define MAX_NAME_SIZE   25

typedef struct player {
    char* name;
    uint32_t score;
} player_t;

player_t* player_create(char* name);
void player_adding_points(player_t* player, uint32_t points);

#endif //PLAYER_H
