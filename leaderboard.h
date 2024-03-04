//
// Created by aless on 03/03/2024.
//

#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string.h>

#include "error_handling.h"

struct Node;
typedef struct Node {
    short id;
    int score, size;
    struct Node* left;
    struct Node* right;
} Node;

Node* root;

inline Node* parent(struct Node* src, const int val, const int parent) {
    if (src == NULL) handle_error("EMPTY_TREE");

    if (src->id != val) {
        parent(src->left, val, src->id);
        parent(src->right, val, src->id);
    }

    return src;
}

inline Node* search(const int score, const short id, Node* src) {
    if (src == NULL) handle_error("EMPTY_TREE");

    if (id < src->id) return search(score, id, src->left);
    if (id > src->id) return search(score, id, src->right);
    return src;
}

inline void new_node (const short id, const int size, const int score) {
    Node* node = malloc(sizeof(Node));
    node->id = id;
    node->size = size;
    node->score = score;

    node->left = NULL;
    node->right = NULL;

    free(node);
}

inline int insert(const Node* root, const short id, const int size, const int score) {
    const int id_diff = root->id - id;

    for(;;) {
        if (0 == id_diff) return 1;

        if (-1 == id_diff) {
            if (NULL == root->right) {
                new_node(id, size, score);
                return 2;
            } else root = root->right;
        } else if (NULL == root->left) {
            new_node(id, size, score);
            return 2;
        } else root = root->left;
    }
}
#endif //LEADERBOARD_H
