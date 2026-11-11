#include <assert.h>
#include <stddef.h>

#include "bostree.h"

static int int_compare(const void *lhs, const void *rhs) {
    const int left = *(const int *)lhs;
    const int right = *(const int *)rhs;
    return (left > right) - (left < right);
}

int main(void) {
    int key = 10;
    int value = 42;

    BOSTree *tree = bostree_new(int_compare, NULL);
    assert(tree != NULL);

    BOSNode *node = bostree_insert(tree, &key, &value);
    assert(node != NULL);
    assert(bostree_node_count(tree) == 1);
    assert(bostree_lookup(tree, &key) == node);
    assert(bostree_select(tree, 0) == node);
    assert(bostree_rank(node) == 0);

    bostree_destroy(tree);
    return 0;
}
