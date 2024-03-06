//
// Created by aless on 06/03/2024.
//
#include <stddef.h>
#include <stdlib.h>

#include "headers/player.h"
#include "headers/player_pool.h"

void handle_collision(const PlayerPool* table, const unsigned long index, Player* item) {
    LinkedList* head = table->overflow_buckets[index];
    if (head == NULL) {
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
    } else {
        table->overflow_buckets[index] = linkedlist_insert(head, item);
    }
}

LinkedList** create_overflow(const PlayerPool* table) {
    LinkedList** buckets = calloc(table->size, sizeof(LinkedList*));
    for (int i = 0; i < table->size; i++) buckets[i] == NULL;
    return buckets;
}

void free_overflow(const PlayerPool* table) {
    LinkedList** buckets = table->overflow_buckets;
    for (int i = 0; i < table->size; i++) free_linkedlist(buckets[i]);
    free(buckets);
}