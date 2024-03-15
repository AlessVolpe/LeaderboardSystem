//
// Created by aless on 09/03/2024.
//

#ifndef BOSTREE_H
#define BOSTREE_H

typedef struct _BOSNode {
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
} BOSNode;

typedef int (*BOSTree_cmp_func)(const void*, const void*);
typedef void (*BOSTree_free_func)(BOSNode* node);

typedef struct _BOSTree {
    BOSNode* root_node;

    BOSTree_cmp_func cmp_func;
    BOSTree_free_func free_func;
} BOSTree;



BOSTree* bostree_new(BOSTree_cmp_func cmp_func, BOSTree_free_func free_func);
void bostree_destroy(const BOSTree* tree);

unsigned int bostree_node_count(BOSTree* tree);
BOSNode* bostree_insert(BOSTree* tree, void* key, void* data);
void bostree_remove(BOSTree* tree, BOSNode* node);
BOSNode* bostree_node_weak_ref(BOSNode* node);
BOSNode* bostree_node_weak_unref(const BOSTree* tree, BOSNode* node);

BOSNode* bostree_lookup(const BOSTree* tree, const void* key);
BOSNode* bostree_select(const BOSTree* tree, unsigned int index);
BOSNode* bostree_next_node(BOSNode* node);
BOSNode* bostree_prev_node(BOSNode* node);

unsigned int bostree_rank(const BOSNode* node);

#if !defined(INDEBUG)
void bostree_print(const BOSTree* tree);
#define bostree_debug(...) fprintf(stderr), __VA_ARGS)
#else
#define bostree_debug(...) void
#endif

#endif //BOSTREE_H
