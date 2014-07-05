#include <stdlib.h>

#include "rb_tree.h"

#define IS_NIL(tree, node) (&((tree)->nil) == (node))

static rb_tree_node_t* rb_tree_search_smallest(rb_tree_t *tree, rb_tree_node_t *node, void *key);
static void rb_tree_delete(rb_tree_t *tree, rb_tree_node_t *z);
static void rb_tree_delete_fixup(rb_tree_t *tree, rb_tree_node_t *x);
static void rb_tree_rotate_left(rb_tree_t *tree, rb_tree_node_t *x);
static void rb_tree_rotate_right(rb_tree_t *tree, rb_tree_node_t *y);
static rb_tree_node_t* rb_tree_successor(rb_tree_t *tree, rb_tree_node_t *node);
static rb_tree_node_t* rb_tree_search(rb_tree_t *tree, rb_tree_node_t *node, void *key);
static void binary_search_tree_insert(rb_tree_t *tree, rb_tree_node_t *z);

rb_tree_t *rb_tree_create(rb_tree_key_cmp_t key_cmp)
{
    rb_tree_t *rb_tree = NULL;

    rb_tree = (rb_tree_t *) calloc(sizeof(rb_tree_t), 1);

    if (NULL == rb_tree) {
        return rb_tree;
    }

    rb_tree->nil.key = NULL;
    rb_tree->nil.count = 0;
    rb_tree->nil.color = BLACK;
    rb_tree->nil.parent = NULL;
    rb_tree->nil.left = NULL;
    rb_tree->nil.right = NULL;

    rb_tree->head = &(rb_tree->nil);
    rb_tree->key_cmp = key_cmp;

    return rb_tree;
}

bool rb_tree_insert(rb_tree_t *tree, void *key, bool *exists)
{
    rb_tree_node_t *x = NULL;
    rb_tree_node_t *y = NULL;
    rb_tree_node_t *new = NULL;

    *exists = false;

    /* First, search for the key. If it is present, all we need to do is to increase its count. */
    x = rb_tree_search(tree, tree->head, key);
    if (NULL != x) {
        *exists = true;
        x->count += 1;
        return true;
    }

    /* If the key doesn't exist, we need to allocate a new node for the key, and actually insert it */
    new = (rb_tree_node_t *) calloc(sizeof(rb_tree_node_t), 1);
    if (NULL == new) {
        return false;
    }
    new->key = key;
    new->count = 1;
    
    /* If the tree is empty, the insertion is pretty easy... */
    if (IS_NIL(tree, tree->head)) {
        new->left = &(tree->nil);
        new->right = &(tree->nil);
        new->parent = NULL;
        new->color = BLACK;
        tree->head = new;

        return true;
    }

    /* TODO: Compare with the book's implementation */
    new->color = RED;

    /* First, insert the new node as a red leaf just like a binary search tree would */
    binary_search_tree_insert(tree, new);
    x = new;
    /* Direct implementation of the book */
    while (x->parent->color == RED) {
        if (x->parent == x->parent->parent->left) {
            y = x->parent->parent->right;
            if (y->color == RED) {
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {
                if (x == x->parent->right) {
                    x = x->parent;
                    rb_tree_rotate_left(tree, x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rb_tree_rotate_right(tree, x->parent->parent);
            }
        } else {
            y = x->parent->parent->left;
            if (y->color == RED) {
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {
                if (x == x->parent->left) {
                    x = x->parent;
                    rb_tree_rotate_right(tree, x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rb_tree_rotate_left(tree, x->parent->parent);
            }
        }
    }
    tree->head->color = BLACK;

    return true;
}

void * rb_tree_fetch_smallest(rb_tree_t *tree, void *key, bool *deleted)
{
    rb_tree_node_t *node = rb_tree_search_smallest(tree, tree->head, key);
    void *found_key = NULL;
    
    *deleted = false;

    if (NULL == node) {
        return NULL;
    }

    found_key = node->key;
    node->count -= 1;
    if (0 == node->count) {
        rb_tree_delete(tree, node);
        *deleted = true;
    }

    return found_key;
}

void rb_tree_in_order(rb_tree_t *tree, rb_tree_node_t *node, void (*callback)(rb_tree_t *tree, rb_tree_node_t *node))
{
    if (IS_NIL(tree, node)) {
        return;
    }

    if (!IS_NIL(tree, node->left)) {
        rb_tree_in_order(tree, node->left, callback);
    }

    callback(tree, node);

    if (!IS_NIL(tree, node->right)) {
        rb_tree_in_order(tree, node->right, callback);
    }
}

/* rb_tree_search_smallest - Search recursively for the smallest node larger than or equal to key.
   An internal function, should be called with tree->head.
   Returns NULL if there are no nodes in the tree or if a larger node is not found.
 */
static rb_tree_node_t* rb_tree_search_smallest(rb_tree_t *tree, rb_tree_node_t *node, void *key)
{
    int compare = 0;

    if (IS_NIL(tree, node)) {
        return NULL;
    }

    compare = tree->key_cmp(key, node->key);

    /* key == node->key */
    if (compare == 0) {
        return node;
    }

    /* key < node->key */
    if (compare < 0) {
        if (IS_NIL(tree, node->left)) {
            return node;
        }
        return rb_tree_search_smallest(tree, node->left, key);
    } else {
        /* key > node->key */
        if (IS_NIL(tree, node->right)) {
            /* The searched key is too large, and there are no more nodes to search for,
               so we have to return NULL */
            return NULL;
        }
        return rb_tree_search_smallest(tree, node->right, key);
    }
}

/* rb_tree_delete - remove a node from the tree. Based on the book's implementation.
   Note that we don't free key; the user is responsible for the keys' memory management.
 */
static void rb_tree_delete(rb_tree_t *tree, rb_tree_node_t *z)
{
    rb_tree_node_t *head = NULL;
    rb_tree_node_t *y = NULL;
    rb_tree_node_t *x = NULL;

    head = tree->head;

    if (IS_NIL(tree, z->left) && IS_NIL(tree, z->right)) {
        y = z;
    } else {
        y = rb_tree_successor(tree, z);
    }

    if (IS_NIL(tree, y->left)) {
        x = y->right;
    } else {
        x = y->left;
    }

    x->parent = y->parent;
    if (head == x->parent) {
        head->left = x;
    } else {
        if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
    }

    if (y != z) {
        if (y->color == BLACK) {
            rb_tree_delete_fixup(tree, x);
        }

        y->left = z->left;
        y->right = z->right;
        y->parent = z->parent;
        y->color = z->color;

        z->left->parent = y;
        z->right->parent = y;

        if (z == z->parent->left) {
            z->parent->left = y;
        } else {
            z->parent->right = y;
        }

        free(z);
    } else {
        if (y->color == BLACK) {
            rb_tree_delete_fixup(tree, x);
        }

        free(y);
    }
}

/* rb_tree_delete_fixup - fixup for the red black structure, taken from the book.
 */
static void rb_tree_delete_fixup(rb_tree_t *tree, rb_tree_node_t *x)
{
    rb_tree_node_t *head = tree->head;
    rb_tree_node_t *w = NULL;

    while ((x->color == BLACK) && (x != head)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_tree_rotate_left(tree, x->parent);
                w = x->parent->right;
            }
            if ((w->right->color == BLACK) && (w->left->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rb_tree_rotate_right(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rb_tree_rotate_left(tree, x->parent);
                x = head;
            }
        } else {
            w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_tree_rotate_right(tree, x->parent);
                w = x->parent->left;
            }
            if ((w->right->color == BLACK) && (w->left->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rb_tree_rotate_left(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rb_tree_rotate_right(tree, x->parent);
                x = head;
            }
        }
    }
    x->color = BLACK;
}

/* rb_tree_rotate_left - a left rotate implementation as shown in the book  */
static void rb_tree_rotate_left(rb_tree_t *tree, rb_tree_node_t *x)
{
    rb_tree_node_t *y = NULL;

    y = x->right;
    x->right = y->left;

    if (!IS_NIL(tree, y->left)) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    /* TODO: Compare to the book, this is taken from the MIT guy's implementation */
    if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

/* rb_tree_rotate_right - a left rotate implementation as shown in the book  */
static void rb_tree_rotate_right(rb_tree_t *tree, rb_tree_node_t *y)
{
    rb_tree_node_t *x = NULL;

    x = y->left;
    y->left = x->right;

    if (!IS_NIL(tree, x->right)) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    /* TODO: Same remark as in left rotate */
    if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

/* TODO: doc */
static rb_tree_node_t* rb_tree_successor(rb_tree_t *tree, rb_tree_node_t *node)
{
    rb_tree_node_t *y = NULL;

    y = node->right;
    if (!IS_NIL(tree, y)) {
        while (!IS_NIL(tree, y->left)) {
            y = y->left;
        }
        return y;
    } else {
        y = node->parent;
        while (node == y->right) {
            node = y;
            y = y->parent;
        }

        if (y == tree->head) {
            return &(tree->nil);
        }

        return y;
    }
}

/* TODO: doc */
static rb_tree_node_t* rb_tree_search(rb_tree_t *tree, rb_tree_node_t *node, void *key)
{
    int compare = 0;
    
    if (IS_NIL(tree, node)) {
        return NULL;
    }

    compare = tree->key_cmp(key, node->key);

    if (0 == compare) {
        return node;
    }

    if (compare < 0) {
        return rb_tree_search(tree, node->left, key);
    } else {
        /* key must be greater than node's key */
        return rb_tree_search(tree, node->right, key);
    }
}

/* binary_search_tree_insert - insert a value to the red-black tree as if it is a binary
   search tree. Based on the book's implementation.
 */
static void binary_search_tree_insert(rb_tree_t *tree, rb_tree_node_t *z)
{
    rb_tree_node_t *x = NULL;
    rb_tree_node_t *y = NULL;
    int compare = 0;

    z->left = &(tree->nil);
    z->right = &(tree->nil);

    y = tree->head;
    x = y;

    while (!IS_NIL(tree, x)) {
        y = x;
        compare = tree->key_cmp(z->key, x->key);

        /* z->key < x->key */
        if (compare < 0) {
            x = x->left;
        } else {
            /* z->key >= x->key */
            x = x->right;
        }
    }

    z->parent = y;

    if (tree->key_cmp(y->key, z->key) > 0) {
        y->left = z;
    } else {
        y->right = z;
    }
}
