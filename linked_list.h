//
// Created by aless on 04/03/2024.
//

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "player.h"

// Defines the LinkedList.
typedef struct LinkedList {
    Player* item;
    struct LinkedList* next;
} LinkedList;;

inline LinkedList* allocate_list() {
    LinkedList* list = malloc(sizeof(LinkedList));
    return list;
}

inline LinkedList* linkedlist_insert(LinkedList* list, Player* item) {
    if (list == NULL) {
        LinkedList* head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }
    if (list->next == NULL) {
        LinkedList* node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    LinkedList* temp = list;
    while (temp->next->next) temp = temp->next;

    LinkedList* node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;
    return list;
}

inline Player* linkedlist_remove(LinkedList* list) {
    if (list == NULL) return NULL;

    if (list->next == NULL) return NULL;

    LinkedList* node = list->next;
    LinkedList* temp = list;
    temp->next = NULL;
    list = node;
    Player* it = NULL;
    memcpy(temp->item, it, sizeof(Player));
    free(temp->item->id);
    free(temp->item->player_name);
    free(temp->item);
    free(temp);
    return it;
}

inline void free_linkedlist(LinkedList* list) {
    while (list) {
        LinkedList *temp = list;
        list = list->next;
        free(temp->item->id);
        free(temp->item->player_name);
        free(temp->item);
        free(temp);
    }
}

inline void free() {

}
#endif //LINKED_LIST_H
