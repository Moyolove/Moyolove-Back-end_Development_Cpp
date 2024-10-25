
#define red   0
#define black 1

//宏定义一个实例，方便代码复用
#define RBTREE_ENTRY(name, type)     \
        struct name                  \
        {                            \      
            /* data */               \
            struct type *left;       \
            struct type *right;      \
            struct type *parent;     \
            unsigned char color;     \   
        };
        

typedef int key_type;

typedef struct _rb_node
{
    /* data */
    key_type key;
    void *value;
#if 1
    struct _rb_node* left;
    struct _rb_node* right;
    struct _rb_node* parent;
    unsigned char color;
#else
    RBTREE_ENTRY(, _rb_node);
#endif

}rb_node;

typedef struct _rbtree
{
    /* data */
    struct _rb_node *root;
    //所有空指针指向同一节点，包括根节点的parent
    struct _rb_node *nil;

    _rbtree(){
        nil->color = black;
    }
}rbtree;

//给一个node变色
void change_color(rb_node *x){
    x->color = x->color == red ? black : red;
}

//左旋x与他的右子树，原子操作，改变3个方向，6根指针
void rbtree_left_rotate(rbtree *T, rb_node* x){
    rb_node *y = x->right;

    x->right = y->left;
    if(y->left != T->nil){
        y->left->parent = x;
    }

    y->parent = x->parent;
    if(x == T->root){
        T->root = y;
    }else if(x->parent->left == x){
        x->parent->left = y;
    }else{
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

//右旋y与他的左子树，原子操作，改变3个方向，6根指针
void rbtree_right_rotate(rbtree *T, rb_node* y){
    rb_node *x = y->left;

    y->left = x->right;
    if(x->right != T->nil){
        x->right->parent = y;
    }

    x->parent = y->parent;
    if(y == T->root){
        T->root = x;
    }else if(y->parent->right == y){
        y->parent->right = x;
    }else{
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

//从z开始修复
void rbtree_insert_fixup(rbtree *T, rb_node *z){
    if(z == T->root){
        z->color = black;
        return;
    }
    
    while(z->parent->color == red){
        rb_node *father = z->parent;
        rb_node *grandfather = father->parent;
        rb_node *uncle = grandfather->left == father ? grandfather->right : grandfather->left;
        //叔叔是红色，叔父爷变色，爷爷变插入节点，继续调整
        if(uncle->color == red){
            change_color(father);
            change_color(grandfather);
            change_color(uncle);
            z = grandfather;
        }else{//叔叔是黑色，旋转+变色
            //LL型旋转
            if(father == grandfather->left && z == father->left){
                rbtree_right_rotate(T, grandfather);
                change_color(father);
                change_color(grandfather);
            }else if(father == grandfather->left && z == father->right){
                //LR型
                rbtree_left_rotate(T, father);
                rbtree_right_rotate(T, grandfather);
                change_color(grandfather);
                change_color(father);
            }//RR
            //RL
        }
    }
}

//插入一个节点z
void rbtree_insert(rbtree *T, rb_node *z){
    rb_node *cur = T->root;
    rb_node *prev = T->nil;
    while(cur != T->nil){
        prev = cur;
        if(z->key < cur->key){
            cur = cur->left;
        }else if(z->key > cur->key){
            cur = cur->right;
        }else{
            //相等时根据业务不同做调整
            return;
        }
    }

    //如果没有节点
    if(prev == T->nil){
        T->root = z;
    }else{
        if(prev->key > z->key){
            prev->left = z;
        }else{
            prev->right = z;
        }
    }

    z->parent = prev;
    z->left = T->nil;
    z->right = T->nil;
    z->color = red;
    rbtree_insert_fixup(T, z);
}