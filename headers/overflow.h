//
// Created by aless on 04/03/2024.
//

#ifndef OVERFLOW_H
#define OVERFLOW_H

#include "player.h"
#include "player_pool.h"

inline void handle_collision(const PlayerPool* table, const unsigned long index, Player* item) {
    LinkedList* head = table->overflow_buckets[index];

    if (head == NULL) {
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
    } else {
        table->overflow_buckets[index] = linkedlist_insert(head, item);
    }
}

inline LinkedList** create_overflow(const PlayerPool* table) {
    LinkedList** buckets = calloc(table->size, sizeof(LinkedList*));
    for (int i = 0; i < table->size; i++) buckets[i] == NULL;
    return buckets;
}

inline void free_overflow(const PlayerPool* table) {
    LinkedList** buckets = table->overflow_buckets;
    for (int i = 0; i < table->size; i++) free_linkedlist(buckets[i]);
    free(buckets);
}
#endif //OVERFLOW_H
