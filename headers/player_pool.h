//
// Created by aless on 04/03/2024.
//

#ifndef PLAYER_POOL_H
#define PLAYER_POOL_H

#include <stdio.h>

#include "error_handling.h"
#include "player.h"
#include "linked_list.h"
#include "overflow.h"

#define CAPACITY 50000;

struct PlayerPool;
typedef struct PlayerPool {
    int size;
    int count;
    Player** players;
    LinkedList** overflow_buckets;
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
    pool->overflow_buckets = create_overflow(pool);
    return pool;
}

inline void free_table(PlayerPool* table) {
    for (int i = 0; i < table->size; i++) {
        Player* item = table->players[i];
        if (item != NULL) free_item(item);
    }
    free_overflow(table);
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

inline void insert(const PlayerPool* table, const char* key, const char* value) {
    Player* item = create_player(key, value);
    const int index = hash_function(key);
    const Player* current_item = table->players[index];

    if (current_item == NULL) {
        if (table->count == table->size) {
            free_item(item);
            handle_error("INSERT_ERR: TABLE_FULL");
        }

        table->players[index] = item;
        table->count++;
    } else {
        if (strcmp(current_item->id, key) == 0) {
            strcpy(table->players[index]->player_name, value);
            exit(EXIT_SUCCESS);
        }
        handle_collision(table, index, item);
    }
}

inline void delete(const PlayerPool* table, const char* key) {
    const int index = hash_function(key);
    Player* item = table->players[index];
    LinkedList* head = table->overflow_buckets[index];

    if (item == NULL) handle_error("ITEM_NULL");
    else {
        if (head == NULL && strcmp(item->id, key) == 0) {
            table->players[index] = NULL;
            free_item(item);
            table->count--;
        } else if (head != NULL) {
            if (strcmp(item->id, key) == 0) {
                free_item(item);
                LinkedList* node = head;
                head = head->next;
                node->next = NULL;
                table->players[index] = create_player(node->item->id, node->item->player_name);
                free_linkedlist(node);
                table->overflow_buckets[index] = head;
            } else {
                LinkedList* curr = head;
                LinkedList* prev = NULL;

                while (curr) {
                    if (strcmp(curr->item->id, key) == 0) {
                        free_linkedlist(head);
                        table->overflow_buckets[index] = NULL;
                    } else {
                        prev->next = curr->next;
                        curr->next = NULL;
                        free_linkedlist(curr);
                        table->overflow_buckets[index] = head;
                    }
                    curr = curr->next;
                    prev = curr;
                }
            }
        }
    }
}

inline char* ht_search(const PlayerPool* table, const char* key) {
    const int index = hash_function(key);
    const Player* item = table->players[index];
    const LinkedList* head = table->overflow_buckets[index];

    if (item != NULL) {
        if (strcmp(item->id, key) == 0) return  item->player_name;
        if (head == NULL) return  NULL;

        item = head->item;
        head = head->next;
    }
    return NULL;
}

inline void print_search(const PlayerPool* table, const char* key) {
    char* val;
    if ((val = ht_search(table, key)) == NULL) handle_error(strcat(
        strcat("Key: ", key), "does not exist\n"));
    else printf("Key:%s, Value:%s\n", key, val);
}

#endif //PLAYER_POOL_H
