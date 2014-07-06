#include <stdlib.h>

#include "rb_tree.h"

#define IS_NIL(tree, node) (&((tree)->nil) == (node))

static rb_tree_node_t* rb_tree_search_smallest(rb_tree_t *tree, rb_tree_node_t *node, void *key);
static void rb_tree_delete(rb_tree_t *tree, rb_tree_node_t *z);
static void rb_tree_delete_fixup(rb_tree_t *tree, rb_tree_node_t *x);
static void rb_tree_insert_fixup(rb_tree_t *tree, rb_tree_node_t *z);
static void rb_tree_rotate_left(rb_tree_t *tree, rb_tree_node_t *x);
static void rb_tree_rotate_right(rb_tree_t *tree, rb_tree_node_t *x);
static rb_tree_node_t* rb_tree_successor(rb_tree_t *tree, rb_tree_node_t *node);
static rb_tree_node_t* rb_tree_search(rb_tree_t *tree, rb_tree_node_t *node, void *key);

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
    rb_tree->nil.parent = &(rb_tree->nil);
    rb_tree->nil.left = &(rb_tree->nil);
    rb_tree->nil.right = &(rb_tree->nil);

    rb_tree->head = &(rb_tree->nil);
    rb_tree->key_cmp = key_cmp;

    return rb_tree;
}

bool rb_tree_insert(rb_tree_t *tree, void *key, bool *exists)
{
    rb_tree_node_t *x = NULL;
    rb_tree_node_t *y = NULL;
    rb_tree_node_t *z = NULL;

    *exists = false;

    /* First, search for the key. If it is present, all we need to do is to increase its count. */
    x = rb_tree_search(tree, tree->head, key);
    if (NULL != x) {
        *exists = true;
        x->count += 1;
        return true;
    }

    /* If the key doesn't exist, we need to allocate a new node for the key, and actually insert it */
    z = (rb_tree_node_t *) calloc(sizeof(rb_tree_node_t), 1);
    if (NULL == z) {
        return false;
    }
    z->key = key;
    z->count = 1;

    y = &(tree->nil);
    x = tree->head;

    while (!IS_NIL(tree, x)) {
        y = x;
        /* z->key < x->key */
        if (tree->key_cmp(z->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (IS_NIL(tree, y)) {
        tree->head = z;
    } else {
        /* z->key < y->key */
        if (tree->key_cmp(z->key, y->key) < 0) {
            y->left = z;
        } else {
            y->right = z;
        }
    }
    z->left = &(tree->nil);
    z->right = &(tree->nil);
    z->color = RED;
    rb_tree_insert_fixup(tree, z);

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
    rb_tree_node_t *y = NULL;
    rb_tree_node_t *x = NULL;

    if (IS_NIL(tree, z->left) || IS_NIL(tree, z->right)) {
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

    if (IS_NIL(tree, y->parent)) {
        tree->head = x;
    } else {
        if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
    }

    if (y != z) {
        /* z->left = y->left; */
        /* z->right = y->right; */
        /* z->parent = y->parent; */
        /* z->color = y->color; */
        z->key = y->key;
        z->count = y->count;
    }

    if (y->color == BLACK) {
        rb_tree_delete_fixup(tree, x);
    }

    free(y);
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

static void rb_tree_insert_fixup(rb_tree_t *tree, rb_tree_node_t *z)
{
    rb_tree_node_t *y = NULL;

    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            y = z->parent->parent->right;
            /* Case #1 */
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                /* Case #2 */
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_tree_rotate_left(tree, z);
                }
                /* Case #3 */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_tree_rotate_right(tree, z->parent->parent);
            }
        } else {
            y = z->parent->parent->left;
            /* Case #1 */
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                /* Case #2 */
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_tree_rotate_right(tree, z);
                }
                /* Case #3 */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_tree_rotate_left(tree, z->parent->parent);
            }
        }
    }
    tree->head->color = BLACK;
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

    if (IS_NIL(tree, x->parent)) {
        tree->head = y;
    } else {
        if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
    }

    y->left = x;
    x->parent = y;
}

/* rb_tree_rotate_right - a left rotate implementation as shown in the book  */
static void rb_tree_rotate_right(rb_tree_t *tree, rb_tree_node_t *x)
{
    rb_tree_node_t *y = NULL;

    y = x->left;
    x->left = y->right;

    if (!IS_NIL(tree, y->right)) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (IS_NIL(tree, x->parent)) {
        tree->head = y;
    } else {
        if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
    }

    y->right = x;
    x->parent = y;
}

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
