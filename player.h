//
// Created by aless on 03/03/2024.
//

#ifndef PLAYER_H
#define PLAYER_H
#include <stdlib.h>
#include <string.h>

struct Player;
typedef struct Player {
    char* id;
    char* player_name;
    int score;
    struct Player* next;
} Player;

inline const Player* plrcmp(const struct Player* p1, const struct Player* p2) {
    register const Player* pc1 = p1;
    register const Player* pc2 = p2;

    const int score1 = pc1->score;
    const int score2 = pc2->score;

    const char* id1 = pc1->id;
    const char* id2 = pc2->id;

    if (score1 > score2) return pc1;
    if (score1 < score2) return pc2;
    if (id1 > id2) return pc1;
    return pc2;
}

inline Player* create_player(const char* id, const char* player_name) {
    Player* item = malloc(sizeof(Player));
    item->id = (char*) malloc(strlen(id) + 1);
    item->player_name = (char*) malloc(strlen(player_name) + 1);
    strcpy(item->id, id);
    strcpy(item->player_name, player_name);

    free(item);
    return  item;
}

inline void free_item(Player* item) {
    free(item->id);
    free(item->player_name);
    free(item);
}

#endif //PLAYER_H
