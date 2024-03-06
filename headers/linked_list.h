//
// Created by aless on 04/03/2024.
//

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "player.h"

typedef struct LinkedList {
    Player* item;
    struct LinkedList* next;
} LinkedList;;

LinkedList* allocate_list();

LinkedList* linkedlist_insert(LinkedList* list, Player* item);

LinkedList* linkedlist_remove(LinkedList* list);

void free_linkedlist(LinkedList* list);

#endif //LINKED_LIST_H
