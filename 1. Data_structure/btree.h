#include<stdlib.h>

typedef struct _btree_node{
    int *keys;
    struct _btree_node **childrens;

    bool isleaf;
    //此节点中有效数据的数量
    int num;
    _btree_node() : keys(nullptr), childrens(nullptr), isleaf(false), num(0){}

}btree_node;

typedef struct _btree
{
    /* data */
    struct _btree_node *root;
    int M;
    _btree(int m) : root(nullptr), M(m){}
}btree;

btree_node* btree_create_node(btree *T, bool leaf){
    btree_node *node = (btree_node*)calloc(1, sizeof(btree_node));

    if(node == nullptr) return NULL;

    node->isleaf = leaf;
    node->keys = (int*)calloc(T->M - 1, sizeof(int));
    node->childrens = (btree_node**)calloc(T->M, sizeof(btree_node*));
    node->num = 0;

    return node;
}
