//
// Created by aless on 04/03/2024.
//

#ifndef PLAYER_POOL_H
#define PLAYER_POOL_H

#include <stdio.h>

#include "player.h"
#define CAPACITY 50000;

struct PlayerPool;
typedef struct PlayerPool {
    Player** players;
    int size, count;
} PlayerPool;

inline unsigned long hash_function(const char* str) {
    unsigned long i = 0;
    for (int j = 0; str[j]; j++) i += str[j];
    return i % CAPACITY;
}

inline PlayerPool* create_pool(const int size) {
    PlayerPool* pool = malloc(sizeof(PlayerPool));
    pool->size = size;
    pool->count = 0;
    pool->players = (Player**) calloc(pool->size, sizeof(Player*));

    for (int i = 0; i < pool->size; i++) pool->players[i] = NULL;
    return pool;
}

inline void free_table(PlayerPool* table) {
    for (int i = 0; i < table->size; i++) {
        Player* item = table->players[i];
        if (item != NULL) free_item(item);
    }

    free(table->players);
    free(table);
}

inline void print_table(const PlayerPool* table) {
    printf("\nPlayer Pool\n-------------------\n");
    for (int i = 0; i < table->size; i++) {
        if (table->players[i]) printf("Index:%d, Key:%s, Value:%s\n",
            i, table->players[i]->id, table->players[i]->player_name);
    }
}

#endif //PLAYER_POOL_H
