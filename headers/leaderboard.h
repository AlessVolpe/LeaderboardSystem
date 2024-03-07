//
// Created by aless on 03/03/2024.
//

#ifndef LEADERBOARD_H
#define LEADERBOARD_H


struct Node;
typedef struct Node {
    char* id;
    int score, size;
    struct Node* left;
    struct Node* right;
} Node;

Node* parent(Node* src, const char* val, const char* parent_id);

Node* tr_search(int score, const char* id, Node* src);

void new_node(char* id, int size, int score);

int lb_insert(const Node* root, char* id, int size, int score);

#endif //LEADERBOARD_H
