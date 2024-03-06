#include <stdio.h>

// #include "headers/player.h"
// #include "headers/leaderboard.h"
#include "headers/id_generation.h"

/*
    int ranking(const int score, const char* id) {
        Node* x = tr_search(score, id, root);
        if (x->id == 0) return -1;

        int r = x->right->size + 1;
        Node* y = x;

        while (y != root) {
            Node* parent_node = parent(y, r, -1);
            if (y == parent_node->left) r += parent_node->right->size + 1;
            y = parent_node;
        }
        return r;
    }
*/

int main(void) {
    printf(generate_uuid());
    return 0;
}
