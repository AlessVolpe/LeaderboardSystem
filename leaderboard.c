//
// Created by aless on 06/03/2024.
//

#include "headers/leaderboard.h"
#include "headers/error_handling.h"

Node* root;

Node* parent(Node* src, const char* val, const char* parent_id) {
    if (src == NULL) handle_error("EMPTY_TREE");
    if (src->id != val) {
        parent(src->left, val, src->id);
        parent(src->right, val, src->id);
    }

    return src;
}

Node* tr_search(const int score, const char* id, Node* src) {
    if (src == NULL) handle_error("EMPTY_TREE");
    if (id < src->id) return tr_search(score, id, src->left);
    if (id > src->id) return tr_search(score, id, src->right);
    return src;
}

void new_node (char* id, const int size, const int score) {
    Node* node = malloc(sizeof(Node));
    node->id = id;
    node->size = size;
    node->score = score;

    node->left = NULL;
    node->right = NULL;

    free(node);
}

int lb_insert(const Node* root, char* id, const int size, const int score) {
    const int id_diff = root->id - id;

    for(;;) {
        if (0 == id_diff) return 1;

        if (-1 == id_diff) {
            if (NULL == root->right) {
                new_node(id, size, score);
                return 2;
            }
            root = root->right;
        } else if (NULL == root->left) {
            new_node(id, size, score);
            return 2;
        } else root = root->left;
    }
}

int ranking(const int score, const char* id) {
    Node* x = tr_search(score, id, root);
    if (x->id == 0) return -1;

    Node* y = x;
    int r = x->right->size + 1;
    while (y != root) {
        char char_arr[100];
        sprintf(char_arr, "%d", r);

        Node* parent_node = parent(y, char_arr, "-1");
        if (y == parent_node->left) r += parent_node->right->size + 1;
        y = parent_node;
    }
    return r;
}