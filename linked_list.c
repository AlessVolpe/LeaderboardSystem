//
// Created by aless on 06/03/2024.
//

#include <stdlib.h>

#include "headers/linked_list.h"

#include <string.h>

#include "headers/error_handling.h"

LinkedList* allocate_list() {
    LinkedList* list = malloc(sizeof(LinkedList));
    return list;
}

LinkedList* linkedlist_insert(LinkedList* list, Player* item) {
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

LinkedList* linkedlist_remove(LinkedList* list) {
    if (list == NULL) handle_error("EMPTY_LIST");

    if (list->next == NULL) handle_error("EMPTY_LIST");

    LinkedList* node = list->next;
    LinkedList* temp = list;
    temp->next = NULL;
    list = node;

    const Player* it = NULL;
    memcpy(temp->item, it, sizeof(Player));
    free(temp->item->id);
    free(temp->item->player_name);
    free(temp->item);
    free(temp);
    return list;
}

void free_linkedlist(LinkedList* list) {
    while (list) {
        LinkedList *temp = list;
        list = list->next;
        free(temp->item->id);
        free(temp->item->player_name);
        free(temp->item);
        free(temp);
    }
}