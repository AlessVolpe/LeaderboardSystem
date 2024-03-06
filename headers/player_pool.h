//
// Created by aless on 04/03/2024.
//

#ifndef PLAYER_POOL_H
#define PLAYER_POOL_H

#include "player.h"
#include "linked_list.h"

#define CAPACITY 50000;

struct PlayerPool;
typedef struct PlayerPool {
    int size;
    static int count;
    Player** players;
    LinkedList** overflow_buckets;
} PlayerPool;

unsigned long hash_function(const char* str);

PlayerPool* create_pool(int size);

void free_table(PlayerPool* table);

void print_table(const PlayerPool* table);

void insert(const PlayerPool* table, const char* key, const char* value);

inline void delete(const PlayerPool* table, const char* key);

char* ht_search(const PlayerPool* table, const char* key);

void print_search(const PlayerPool* table, const char* key);

#endif //PLAYER_POOL_H
