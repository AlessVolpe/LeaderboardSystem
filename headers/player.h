//
// Created by aless on 03/03/2024.
//

#ifndef PLAYER_H
#define PLAYER_H

struct Player;
typedef struct Player {
    char* id;
    char* player_name;
    int score;
    struct Player* next;
} Player;

const Player* plrcmp(const struct Player* p1, const struct Player* p2);

Player* create_player(const char* id, const char* player_name);

void free_item(Player* item);

#endif //PLAYER_H
