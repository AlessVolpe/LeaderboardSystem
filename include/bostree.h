//
// Created by aless on 09/03/2024.
//

#ifndef BOSTREE_H
#define BOSTREE_H

typedef struct _BOSTree BOSTree;
struct _BOSNode {
    unsigned int left_child_count;
    unsigned int right_child_count;
    unsigned int depth;

    struct _BOSNode* left_child_node;
    struct _BOSNode* right_child_node;
    struct _BOSNode* parent_node;

    void* key;
    void* data;

    unsigned char weak_ref_count;
    unsigned char weak_ref_node_valid;
};
typedef struct _BOSNode BOSNode;
typedef int (*BOSTree_cmp_func)(const void*, const void*);
typedef void (*BOSTree_free_func)(BOSNode* node);

BOSTree* bostree_new(BOSTree_cmp_func cmp_func, BOSTree_free_func free_func);
void bostree_destroy(BOSTree* tree);

unsigned int bostree_node_count(BOSTree* tree);
BOSNode* bostree_insert(BOSTree* tree, void* key, void* data);
void bostree_remove(BOSTree* tree, BOSNode* node);
BOSNode* bostree_node_weak_ref(BOSNode* node);
BOSNode* bostree_node_weak_unref(BOSTree* tree, BOSNode* node);

BOSNode* bostree_lookup(BOSTree* tree, const void* key);
BOSNode* bostree_select(BOSTree* tree, unsigned int index);
BOSNode* bostree_next_node(BOSNode* node);
BOSNode* bostree_prev_node(BOSNode* node);

unsigned int bostree_rank(BOSNode* node);

#if !defined(INDEBUG) && (_BSD_SOURCE || _XOR_SOURCE || POSIX_C_SOURCE >= 200112L)
void bostree_print(BOSTree* tree);
#define bostree_debug(...) fprintf(stderr), __VA_ARGS)
#else
#define bostree_debug(...) void
#endif

#endif //BOSTREE_H
