#include <stdio.h>

#include "player.h";
#include "leaderboard.h";

int ranking(const int score, const short id) {
    Node* x = search(score, id, root);
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

int main(void) {
    Player* player1 = malloc(sizeof(Player));
    player1->id = 1;
    player1->score = 255;

    int rank = ranking(player1->score, player1->id);

    free(player1);
    return 0;
}
