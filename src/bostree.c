//
// Created by aless on 09/03/2024.
//

#include "bostree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commons.h"

/*
 * local helper function
 */
static int _imax(const int i1, const int i2) {
    return i1 > i2 ? i1 : i2;
}

static int _bostree_balance(const BOSNode* node) {
    const int left_depth = node->left_child_node ? node->left_child_node->depth + 1 : 0;
    const int right_depth = node->right_child_node ? node->right_child_node->depth + 1 : 0;
    return right_depth - left_depth;
}

static BOSNode* _bostree_rotate_right(BOSTree* tree, BOSNode* parent) {
    BOSNode* left = parent->left_child_node;

    if (parent->parent_node == NULL) tree->root_node = parent->left_child_node;
    else {
        if (parent->parent_node->left_child_node == parent) parent->parent_node->left_child_node = left;
        else parent->parent_node->right_child_node = left;
    }

    left->parent_node = parent->parent_node;
    left = parent->left_child_node = left->left_child_node;
    parent->left_child_count = left->left_child_count;

    if (parent->left_child_node) parent->left_child_node->parent_node = parent;

    parent->depth = _imax(parent->left_child_node ? 1 + parent->left_child_node->depth : 0,
        parent->right_child_node ? 1 + parent->right_child_node->depth : 0);

    parent->parent_node = left;
    left->right_child_node = parent;
    parent->parent_node = left;
    left->right_child_count = parent->left_child_count + parent->right_child_count + 1;
    left->depth = _imax(left->left_child_node ? 1 + left->left_child_node->depth : 0,
        left->right_child_node ? 1 + left->right_child_node->depth : 0);

    return left;
}

static BOSNode* _bostree_rotate_left(BOSTree* tree, BOSNode* parent) {
    BOSNode* right = parent->right_child_node;

    if (parent->parent_node == NULL) tree->root_node = right;
    else {
        if (parent->parent_node->left_child_node == parent) parent->parent_node->left_child_node =  right;
        else parent->parent_node->right_child_node = right;
    }

    right->parent_node = parent->parent_node;

    parent->right_child_node = right->left_child_node;
    parent->right_child_count = right->left_child_count;
    if (parent->right_child_node) parent->right_child_node->parent_node = parent;
    parent->depth = _imax(parent->left_child_node ? 1 + parent->left_child_node->depth : 0,
        parent->right_child_node ? 1 + parent->right_child_node->depth : 0);
    parent->parent_node = right;

    right->left_child_node = parent;
    parent->parent_node = right;
    right->left_child_count = parent->left_child_count + parent->right_child_count + 1;
    right->depth = _imax(right->left_child_node ? 1 + right->left_child_node->depth : 0,
        right->right_child_node ? 1 + right->right_child_node->depth : 0);
    return right;
}

static BOSNode* _bostree_new_node(BOSNode* parent_node, void* key, void* data) {
    BOSNode* new_node = malloc(sizeof(BOSNode));
    memset(new_node, 0, sizeof(BOSNode));
    new_node->key = key;
    new_node->data = data;
    new_node->weak_ref_count = 1;
    new_node->weak_ref_node_valid = 1;
    new_node->parent_node = parent_node;
    return new_node;
}

BOSTree* bostree_new(const BOSTree_cmp_func cmp_func, const BOSTree_free_func free_func) {
    BOSTree* new_tree = malloc(sizeof(BOSTree));
    new_tree->root_node = NULL;
    new_tree->cmp_func = cmp_func;
    new_tree->free_func = free_func;
    return new_tree;
}

void bostree_destroy(BOSTree* tree) {
    BOSNode* node = tree->root_node;
    while (node) {
        while (node->left_child_node) node = node->left_child_node;
        if (node->right_child_node) {
            node = node->right_child_node;
            continue;
        }

        BOSNode* next = node->parent_node;
        if (next) {
            if (next->left_child_node == node) next->left_child_node = NULL;
            else next->right_child_node = NULL;
        }
        bostree_node_weak_unref(tree, node);
        node = next;
    }
    free(tree);
}

unsigned int bostree_node_count(const BOSTree* tree) {
    return tree->root_node ? tree->root_node->left_child_count + tree->root_node->right_child_count + 1 : 0;
}

BOSNode* bostree_insert(BOSTree* tree, void* key, void* data) {
    BOSNode** node = &tree->root_node;
    BOSNode* parent_node = NULL;

    while  (*node) {
        parent_node = *node;
        const int cmp = tree->cmp_func(key, (*node)->key);
        if (cmp < 0) {
            (*node)->left_child_count++;
            node = &(*node)->left_child_node;
        } else {
            (*node)->right_child_count++;
            node = &(*node)->right_child_node;
        }
    }

    BOSNode* new_node = _bostree_new_node(parent_node, key, data);
    *node = new_node;

    if (parent_node == NULL) {
        tree->root_node = new_node;
        return new_node;
    }

    if (!!(u_int64)parent_node->left_child_node ^ !!(u_int64)parent_node->right_child_node) {
        parent_node->depth++;
        while (parent_node->parent_node) {
            parent_node = parent_node->parent_node;


            const unsigned int new_left_depth  = parent_node->left_child_node ?
                parent_node->left_child_node->depth + 1 : 0;
            const unsigned int new_right_depth = parent_node->right_child_node ?
                parent_node->right_child_node->depth + 1: 0;

            const unsigned int max_depth = _imax(new_left_depth, new_right_depth);
            if (parent_node->depth != max_depth) parent_node->depth = max_depth;
            else break;

            if (new_left_depth - 2 == new_right_depth) {
                if (_bostree_balance(parent_node->left_child_node) > 0)
                    _bostree_rotate_left(tree, parent_node->left_child_node);
                parent_node = _bostree_rotate_right(tree, parent_node);
            } else if (new_left_depth + 2 == new_right_depth) {
                if(_bostree_balance(parent_node->right_child_node) < 0)
                    _bostree_rotate_right(tree, parent_node->right_child_node);
                parent_node = _bostree_rotate_left(tree, parent_node);
            }
        }
    }
    return new_node;
}

void bostree_remove(BOSTree* tree, BOSNode* node) {
    handle_error("REMOVE_ERR: NOT_IMPLEMENTED");
}

BOSNode* bostree_node_weak_ref(BOSNode* node) {
    assert(node->weak_ref_count < 127);
    assert(node->weak_ref_count > 0);
    node->weak_ref_count++;
    return node;
}

BOSNode* bostree_node_weak_unref(const BOSTree* tree, BOSNode* node) {
    node->weak_ref_count--;
    if (node->weak_ref_count == 0) {
        if (tree->free_func) tree->free_func(node);
        free(node);
    } else if (node->weak_ref_node_valid) return node;
    return NULL;
}

BOSNode* bostree_lookup(const BOSTree* tree, const void* key) {
    BOSNode* node = tree->root_node;
    while (node) {
        const int cmp = tree->cmp_func(key, node->key);
        if (cmp == 0) break;
        if (cmp < 0) node = node->left_child_node;
        else node = node->right_child_node;
    }
    return node;
}

BOSNode* bostree_select(const BOSTree* tree, unsigned int index) {
    BOSNode* node = tree->root_node;

    while (node) {
        if (node->left_child_count > index) node = node->left_child_node;
        else {
            index -= node->left_child_count;
            if (index == 0) return node;
            index--;
            node = node->right_child_node;
        }
    }
    return node;
}

BOSNode* bostree_next_node(BOSNode* node) {
    if (node->right_child_node) {
        node = node->right_child_node;
        while (node->left_child_node) node = node->left_child_node;
        return node;
    }

    if (node->parent_node) {
        while(node->parent_node && node->parent_node->right_child_node == node) node = node->parent_node;
        return node->parent_node;
    }
    return NULL;
}

BOSNode* bostree_prev_node(BOSNode* node) {
    if (node->left_child_node) {
        node = node->left_child_node;
        while (node->right_child_node) node = node->right_child_node;
        return node;
    }

    if (node->parent_node) {
        while(node->parent_node && node->parent_node->left_child_node == node) node = node->parent_node;
        return node->parent_node;
    }
    return NULL;
}

unsigned int bostree_rank(const BOSNode* node) {
    unsigned int counter = node->left_child_count;
    while (node) {
        if (node->parent_node && node->parent_node->right_child_node == node)
            counter += node->parent_node->left_child_count;
        node = node->parent_node;
    }
    return counter;
}

#if !defined(NDEBUG)

/*
 * Debug helpers: print tree to stdout in dot format
 */

static  void _bostree_print_helper(const BOSNode* node) {
    printf("  %s [label=\"\\N (%d,%d,%d)\"];\n", (char*)node->key, node->left_child_count,
        node->right_child_count, node->depth);
    if(node->parent_node) printf("  %s -> %s [color=green];\n", (char*)node->key, (char*)node->parent_node->key);
    if(node->left_child_node != NULL) {
        printf("  %s -> %s\n", (char*)node->key, (char*)node->left_child_node->key);
        _bostree_print_helper(node->left_child_node);
    }
    if(node->right_child_node != NULL) {
        printf("  %s -> %s\n", (char*)node->key, (char*)node->right_child_node->key);
        _bostree_print_helper(node->right_child_node);
    }
}

void bostree_print(const BOSTree* tree) {
    GENERIC_ERROR_HELPER((tree->root_node == NULL), errno, "PRINT_ERR: EMPTY_TREE");

    printf("digraph {\n  ordering = out;\n");
    _bostree_print_helper(tree->root_node);
    printf("}\n");
    FlushFileBuffers(0);
}

#endif
