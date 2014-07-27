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
    rb_tree_node_t *max;
    unsigned int count;
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

   The node insertion's implementation is based on the book's implementation.
 */
bool rb_tree_insert(rb_tree_t *tree, void *key, bool *exists);

/* rb_tree_remove - Remove the given tree from the key.
   If the key exists, its reference count is decreased. If it is decreased to zero, it is removed
   from the tree and deleted would contain a pointer to the key is returned (so that it can be freed),
   or NULL otherwise. In this case, true is returned.
   If the key doesn't exist, false is returned.
 */
bool rb_tree_remove(rb_tree_t *tree, void *key, void **deleted);

/* rb_tree_fetch_smallest - Search for the smallest key that is larger than or equal to the given key.
   If there's no key larger than or equal to the key, NULL is returned.
   If an allocation error occurs, NULL is returned.
 */
rb_tree_node_t* rb_tree_search_smallest(rb_tree_t *tree, void *key);

/* rb_tree_in_order - scan the tree in order and call callback for each node, starting from the given node.
   In order to scan the entire tree, pass tree->head in node.
 */
void rb_tree_in_order(rb_tree_t *tree, rb_tree_node_t *node, void (*callback)(rb_tree_t *tree, rb_tree_node_t *node));

/* rb_tree_find_max - returns the max node in the tree. */
rb_tree_node_t* rb_tree_find_max(rb_tree_t *tree);

/* rb_tree_search - an exact key search in the tree. An equal key in the tree is returned
   or NULL if not found. */
void* rb_tree_search(rb_tree_t *tree, void *key);

/* rb_tree_successor - get the successor in the tree for node. Based on the book's implementation. */
rb_tree_node_t* rb_tree_successor(rb_tree_t *tree, rb_tree_node_t *node);

#endif /* __RB_TREE_H__ */
