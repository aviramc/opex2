/*
  Red-Black tree by custom keys.
  Each key holds the number of instances it has. When a key is removed, this reference count
  is decreased up to 0. When no instances are left, the key is actually removed.
*/

#include <stdbool.h>

#ifndef __RB_TREE_H__
#define __RB_TREE_H__

/* Key compare function - gets key a and key and returns:
      0 - If the keys are equal
     -1 - If a < b
      1 - if a > b
 */
typedef int (* rb_tree_key_cmp_t)(void *a, void *b);

typedef struct rb_tree_node_s rb_tree_node_t;

typedef enum rb_tree_color_s {
    BLACK = 0,
    RED = 1,
} rb_tree_color_t;

struct rb_tree_node_s {
    void *key;
    unsigned int count;
    rb_tree_node_t *parent;
    rb_tree_node_t *left;
    rb_tree_node_t *right;
    rb_tree_color_t color;
};

typedef struct rb_tree_s {
    rb_tree_node_t *head;
    rb_tree_node_t nil;
    rb_tree_key_cmp_t key_cmp;
} rb_tree_t;

/* rb_tree_create - Create an RB tree instance.
   Paramteres:
     - key_cmp - A function for comparison between the keys.
   The function allocates and initializes an empty tree, and returns it.
   If an allocation error occurs, NULL is returned.
 */
rb_tree_t *rb_tree_create(rb_tree_key_cmp_t key_cmp);

/* rb_tre_insert - Inserts a key to the tree.
   If the key already exists, its reference count is increased, and exists would be true,
   so that one would know wether to free or not the key.
   Returns false if an allocation fails, true otherwise.
 */
bool rb_tree_insert(rb_tree_t *tree, void *key, bool *exists);

/* rb_tree_fetch_smallest - Fetch the smallest key that is larger than or equal to the given key.
   If there's no key larger than or equal to the key, NULL is returned.
   The key is removed from the tree, and its count is reduces.
   When the key's count is zero, it is removed from the tree, and deleted would contain true, so
   that one would be able to free the key.
 */
void * rb_tree_fetch_smallest(rb_tree_t *tree, void *key, bool *deleted);

/* rb_tree_in_order - scan the tree in order and call callback for each node.
   Should be initially called with tree->head.
 */
void rb_tree_in_order(rb_tree_t *tree, rb_tree_node_t *node, void (*callback)(rb_tree_t *tree, rb_tree_node_t *node));

#endif /* __RB_TREE_H__ */
