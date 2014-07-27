#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "rb_tree.h"
#include "box_factory.h"

/* box_factory_insert_tree_by_side, box_factory_insert_tree_by_height - insertion functions for the
   two main trees.
 */
static bool box_factory_insert_tree_by_side(box_factory_t *factory, unsigned int side, unsigned int height);
static bool box_factory_insert_tree_by_height(box_factory_t *factory, unsigned int side, unsigned int height);

/* box_factory_remove_tree_by_side, box_factory_remove_tree_by_height - removal functions for the
   two main trees.
 */
static bool box_factory_remove_tree_by_side(box_factory_t *factory, unsigned int side, unsigned int height);
static bool box_factory_remove_tree_by_height(box_factory_t *factory, unsigned int side, unsigned int height);

/* create_box_key - key creation function for the trees. The key would be the same, but the comparison
   function would be different. Returns NULL on an allocation failure.
 */
static box_key_t* create_box_key(unsigned int subval);

/* create_main_tree_node - creates a node in a main tree - tree_by_side or tree_by_height.
   Returns NULL on an allocation failure.
 */
static box_main_tree_node_t* create_main_tree_node(unsigned int main_val, rb_tree_key_cmp_t key_cmp);

/* free_main_tree_node - frees an allocated main tree node, assuming that its subtree is empty */
static void free_main_tree_node(box_main_tree_node_t *node);

/* compare_nodes_by_side, compare_nodes_by_height - node comparison functions for the main trees
   of the box factory */
static int compare_nodes(void *a, void *b);

/* compare_keys_side, compare_keys_height - key comparison functions for the subtrees of each node
   in the main trees.
 */
static int compare_keys(void *a, void *b);

/* The following are convenience functions internally used:
      get_sub_tree - returns the subtree of a given main tree node.
      get_sub_tree_max - returns the max value in the sub tree of a main tree node.
      get_main_tree_node_val - returns the key value of a main tree node.
      get_sub_tree_node_val - returns the key value of a sub tree node.
 */
static rb_tree_t * get_sub_tree(rb_tree_node_t *main_tree_node);
static unsigned int get_sub_tree_max(rb_tree_node_t *main_tree_node);
static unsigned int get_main_tree_node_val(rb_tree_node_t *main_tree_node);
static unsigned int get_sub_tree_node_val(rb_tree_node_t *sub_tree_node);

/* The following are check_box and get_box implementations that can be called on either of the
   two main trees, and are general. The real get_box and check_box would call directly to these
   functions with the main tree that is smaller.
 */
static bool box_factory_check_by_input(rb_tree_t *tree, unsigned int main_val, unsigned int sub_val);
static bool box_factory_get_by_input(rb_tree_t *tree,
                                     unsigned int main_val,
                                     unsigned int sub_val,
                                     unsigned int *found_main_val,
                                     unsigned int *found_sub_val);


box_factory_t* box_factory_create()
{
    box_factory_t *factory = NULL;
    rb_tree_t *rb_tree = NULL;

    factory = calloc(sizeof(box_factory_t), 1);
    if (NULL == factory) {
        return NULL;
    }

    rb_tree = rb_tree_create((rb_tree_key_cmp_t) compare_nodes);
    if (NULL == rb_tree) {
        free(factory);
        return NULL;
    }
    factory->tree_by_side = rb_tree;

    rb_tree = rb_tree_create((rb_tree_key_cmp_t) compare_nodes);
    if (NULL == rb_tree) {
        free(factory->tree_by_side);
        free(factory);
        return NULL;
    }
    factory->tree_by_height = rb_tree;

    return factory;
}

bool box_factory_insert(box_factory_t *factory, unsigned int side, unsigned int height)
{
    if (false == box_factory_insert_tree_by_side(factory, side, height)) {
        return false;
    }

    if (false == box_factory_insert_tree_by_height(factory, side, height)) {
        assert(box_factory_remove_tree_by_side(factory, side, height));
        return false;
    }

    return true;
}

bool box_factory_remove(box_factory_t *factory, unsigned int side, unsigned int height)
{
    if (false == box_factory_remove_tree_by_side(factory, side, height)) {
        return false;
    }

    /* If we were able to remove from the tree by side, we must be able to remove
       from the tree by height, because the key exists.
     */
    assert(box_factory_remove_tree_by_height(factory, side, height));

    return true;
}


bool box_factory_get_box(box_factory_t *factory, unsigned int side, unsigned int height, unsigned int *found_side_square, unsigned int *found_height)
{
    if (factory->tree_by_height->count == 0) {
        return false;
    }

    if (factory->tree_by_height->count > factory->tree_by_side->count){
        return box_factory_get_by_input(factory->tree_by_side, side * side, height, found_side_square, found_height);
    }
    return box_factory_get_by_input(factory->tree_by_height, height, side * side, found_height, found_side_square);
}

static bool box_factory_get_by_input(rb_tree_t *tree,
                                     unsigned int main_val,
                                     unsigned int sub_val,
                                     unsigned int *found_main_val,
                                     unsigned int *found_sub_val)
{
    rb_tree_node_t *node = NULL;
    rb_tree_node_t *sub_node = NULL;
    rb_tree_node_t *min_node = NULL;
    rb_tree_node_t *min_sub_node = NULL;
    box_main_tree_node_t target_node = {.val = main_val, .subtree = NULL};
    box_key_t target_subnode = {.val = sub_val};
    unsigned int min_volume = 0;
    unsigned int volume = 0;

    node = rb_tree_search_smallest(tree, &target_node);

    while (node && (get_sub_tree_max(node) < sub_val)) {
        node = rb_tree_successor(tree, node);
    }

    if (NULL == node) {
        return NULL;
    }

    sub_node = rb_tree_search_smallest(get_sub_tree(node), &target_subnode);

    /* sub_node must exist in this flow. */
    assert(sub_node);

    min_volume = get_main_tree_node_val(node) * get_sub_tree_node_val(sub_node);
    min_node = node;
    min_sub_node = sub_node;

    while (node && (min_volume / get_main_tree_node_val(node) >= sub_val)) {
        node = rb_tree_successor(tree, node);

        if ((NULL == node) || (get_sub_tree_max(node) < sub_val)) {
            continue;
        }

        sub_node = rb_tree_search_smallest(get_sub_tree(node), &target_subnode);

        volume = get_main_tree_node_val(node) * get_sub_tree_node_val(sub_node);
        if (min_volume > volume) {
            min_volume = volume;
            min_node = node;
            min_sub_node = sub_node;
        }
    }

    *found_main_val = get_main_tree_node_val(min_node);
    *found_sub_val = get_sub_tree_node_val(min_sub_node);

    return true;
}


bool box_factory_check_box(box_factory_t *factory, unsigned int side, unsigned int height)
{
    if (factory->tree_by_height->count > factory->tree_by_side->count){
        return box_factory_check_by_input(factory->tree_by_side, side * side, height);
    }
    return box_factory_check_by_input(factory->tree_by_height, height, side * side);
}

static bool box_factory_check_by_input(rb_tree_t *tree, unsigned int main_val, unsigned int sub_val)
{
    rb_tree_node_t *main_node = NULL;
    box_main_tree_node_t *main_key = NULL;
    box_main_tree_node_t target_node;
    box_key_t target_subnode;

    target_node.val = main_val;
    target_subnode.val = sub_val;
    main_node = rb_tree_search_smallest(tree, &target_node);

    if (NULL == main_node){
        return false;
    }

    main_key = (box_main_tree_node_t*)main_node->key;
    while ((NULL != main_node) && (-1 == main_key->subtree->key_cmp(main_key->subtree->max->key, &target_subnode))){
        main_node = rb_tree_successor(tree, main_node);
        if (NULL != main_node){
            main_key = (box_main_tree_node_t*)(main_node->key);
        }
    }

    if (NULL == main_node){
        return false;
    }

    return true;
}

static bool box_factory_insert_tree_by_side(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_main_tree_node_t *new_node = NULL;
    box_main_tree_node_t *side_tree_node = NULL;
    box_key_t *new_key = NULL;
    bool exists_in_side_tree = false;
    bool exists_in_subtree = false;

    /* When trying to insert a new node to this tree, one of the following will cases occur:
         1. There's no box of the same size, meaning the side would not be found in the tree by side.
         2. There's a box with the same side, but not with the same height.
         3. There's a box with the same side and with the same height.
     */
    new_node = create_main_tree_node(side * side, compare_keys);
    if (NULL == new_node) {
        return false;
    }

    new_key = create_box_key(height);
    if (NULL == new_key) {
        return false;
    }

    /* First, search in the main tree (tree by side) */
    side_tree_node = rb_tree_search(factory->tree_by_side, new_node);

    /* Case 1 - there's no box of the same size */
    if (NULL == side_tree_node) {
        /* Insert to the tree by side - this must be a new key in the tree. */
        if (false == rb_tree_insert(factory->tree_by_side, new_node, &exists_in_side_tree)) {
            free(new_node);
            free(new_key);
            return false;
        }
        assert(exists_in_side_tree == false);

        /* Insert to the subtree - this must be a new key in the tree. */
        if (false == rb_tree_insert(new_node->subtree, new_key, &exists_in_subtree)) {
            rb_tree_remove(factory->tree_by_side, new_node, (void **) &side_tree_node);
            assert(side_tree_node == new_node);
            free_main_tree_node(new_node);
            free(new_key);
            return false;
        }
        assert(exists_in_subtree == false);
        return true;
    }

    /* There's a node with the same side */
    free_main_tree_node(new_node);

    /* Now rb_tree_insert should take care of cases 2 & 3. */
    if (false == rb_tree_insert(side_tree_node->subtree, new_key, &exists_in_subtree)) {
        free(new_key);
        return false;
    }

    /* This is case 3, in which we can free the new key that we've created */
    if (exists_in_subtree) {
        free(new_key);
    }

    return true;
}

static bool box_factory_insert_tree_by_height(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_main_tree_node_t *new_node = NULL;
    box_main_tree_node_t *height_tree_node = NULL;
    box_key_t *new_key = NULL;
    bool exists_in_height_tree = false;
    bool exists_in_subtree = false;

    /* When trying to insert a new node to this tree, one of the following will cases occur:
         1. There's no box of the same size, meaning the height would not be found in the tree by height.
         2. There's a box with the same height, but not with the same side.
         3. There's a box with the same height and with the same side.
     */
    new_node = create_main_tree_node(height, compare_keys);
    if (NULL == new_node) {
        return false;
    }

    /* First, search in the main tree (tree by height) */
    height_tree_node = rb_tree_search(factory->tree_by_height, new_node);

    new_key = create_box_key(side * side);
    if (NULL == new_key) {
        return false;
    }

    /* Case 1 - there's no box of the same size */
    if (NULL == height_tree_node) {
        /* Insert to the tree by side - this must be a new key in the tree. */
        if (false == rb_tree_insert(factory->tree_by_height, new_node, &exists_in_height_tree)) {
            free(new_node);
            free(new_key);
            return false;
        }
        assert(exists_in_height_tree == false);

        /* Insert to the subtree - this must be a new key in the tree. */
        if (false == rb_tree_insert(new_node->subtree, new_key, &exists_in_subtree)) {
            rb_tree_remove(factory->tree_by_height, new_node, (void **) &height_tree_node);
            assert(height_tree_node == new_node);
            free_main_tree_node(new_node);
            free(new_key);
            return false;
        }
        assert(exists_in_subtree == false);
        return true;
    }

    /* There's a node with the same height */
    free_main_tree_node(new_node);

    /* Now rb_tree_insert should take care of cases 2 & 3. */
    if (false == rb_tree_insert(height_tree_node->subtree, new_key, &exists_in_subtree)) {
        free(new_key);
        return false;
    }

    /* This is case 3, in which we can free the new key that we've created */
    if (exists_in_subtree) {
        free(new_key);
    }

    return true;
}


static bool box_factory_remove_tree_by_side(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_main_tree_node_t *new_node = NULL;
    box_main_tree_node_t *side_tree_node = NULL;
    box_main_tree_node_t *deleted_side_tree_node = NULL;
    box_key_t *new_key = NULL;
    box_key_t *subtree_key = NULL;

    /* When trying to remove a node from this tree, the following cases may occur:
        1. There's no box with that size, which either means:
            1.1. There's no box with the same side.
            1.2. There's a box with the same side but not with the same height.
        2. There's a box with the same size. If there's only one, we should remove the node from the tree.
     */
    new_node = create_main_tree_node(side * side, compare_keys);
    if (NULL == new_node) {
        return false;
    }

    /* First, search in the main tree (tree by side) */
    side_tree_node = rb_tree_search(factory->tree_by_side, new_node);

    if (NULL == side_tree_node) {
        /* Case 1.1 */
        free_main_tree_node(new_node);
        return false;
    }
    free_main_tree_node(new_node);

    new_key = create_box_key(height);
    if (NULL == new_key) {
        return false;
    }

    subtree_key = rb_tree_search(side_tree_node->subtree, new_key);
    if (NULL == subtree_key) {
        /* Case 1.2 */
        free(new_key);
        return false;
    }

    /* Case 2 */
    /* Remove the node from the subtree */
    assert(rb_tree_remove(side_tree_node->subtree, new_key, (void **) &subtree_key));
    if (subtree_key) {
        /* There's no more of the same side and height in the tree, free the key */
        free(subtree_key);
    }

    /* The subtree has been emptied, so the node should be completely removed */
    if (side_tree_node->subtree->count == 0) {
        assert(rb_tree_remove(factory->tree_by_side, side_tree_node, (void **) &deleted_side_tree_node));
        /* This must be the same node. */
        assert(deleted_side_tree_node == side_tree_node);
        free_main_tree_node(deleted_side_tree_node);
    }
    free(new_key);
    return true;
}

static bool box_factory_remove_tree_by_height(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_main_tree_node_t *new_node = NULL;
    box_main_tree_node_t *height_tree_node = NULL;
    box_main_tree_node_t *deleted_height_tree_node = NULL;
    box_key_t *new_key = NULL;
    box_key_t *subtree_key = NULL;

    /* When trying to remove a node from this tree, the following cases may occur:
        1. There's no box with that size, which either means:
            1.1. There's no box with the same height.
            1.2. There's a box with the same height but not with the same side.
        2. There's a box with the same size. If there's only one, we should remove the node from the tree.
     */
    new_node = create_main_tree_node(height, compare_keys);
    if (NULL == new_node) {
        return false;
    }

    /* First, search in the main tree (tree by side) */
    height_tree_node = rb_tree_search(factory->tree_by_height, new_node);

    if (NULL == height_tree_node) {
        /* Case 1.1 */
        free_main_tree_node(new_node);
        return false;
    }
    free_main_tree_node(new_node);

    new_key = create_box_key(side * side);
    if (NULL == new_key) {
        return false;
    }

    subtree_key = rb_tree_search(height_tree_node->subtree, new_key);
    if (NULL == subtree_key) {
        /* Case 1.2 */
        free(new_key);
        return false;
    }

    /* Case 2 */
    /* Remove the node from the subtree */
    assert(rb_tree_remove(height_tree_node->subtree, new_key, (void **) &subtree_key));
    if (subtree_key) {
        /* There's no more of the same height and side in the tree, free the key */
        free(subtree_key);
    }

    /* The subtree has been emptied, so the node should be completely removed */
    if (height_tree_node->subtree->count == 0) {
        assert(rb_tree_remove(factory->tree_by_height, height_tree_node, (void **) &deleted_height_tree_node));
        /* This must be the same node. */
        assert(deleted_height_tree_node == height_tree_node);
        free_main_tree_node(deleted_height_tree_node);
    }
    free(new_key);
    return true;
}

static box_key_t* create_box_key(unsigned int subval)
{
    box_key_t *key = calloc(sizeof(box_key_t), 1);
    if (NULL == key) {
        return NULL;
    }

    key->val = subval;

    return key;
}

static box_main_tree_node_t* create_main_tree_node(unsigned int main_val, rb_tree_key_cmp_t key_cmp)
{
    box_main_tree_node_t *node = calloc(sizeof(box_main_tree_node_t), 1);
    rb_tree_t *subtree = NULL;

    if (NULL == node) {
        return NULL;
    }

    node->val = main_val;

    subtree = rb_tree_create((rb_tree_key_cmp_t) compare_keys);
    if (NULL == subtree) {
        free(node);
        return NULL;
    }
    node->subtree = subtree;

    return node;
}

static void free_main_tree_node(box_main_tree_node_t *node)
{
    /* XXX: We assume that the subtree is empty */
    free(node->subtree);
    free(node);
}

static int compare_nodes(void *a, void *b)
{
    /* Compare nodes of the tree_by_side */
    box_main_tree_node_t *node_a = a;
    box_main_tree_node_t *node_b = b;

    if (node_a->val < node_b->val) {
        return -1;
    }

    if (node_a->val > node_b->val) {
        return 1;
    }

    /* The sides of the two nodes are equal */
    return 0;
}

static int compare_keys(void *a, void *b)
{
    /* Compare nodes of the subtree of a node in tree_by_height */
    box_key_t *key_a = a;
    box_key_t *key_b = b;

    if (key_a->val < key_b->val) {
        return -1;
    }

    if (key_a->val > key_b->val) {
        return 1;
    }

    /* The sides of the two keys are equal */
    return 0;
}

static rb_tree_t * get_sub_tree(rb_tree_node_t *main_tree_node)
{
    box_main_tree_node_t *main_tree_key = NULL;

    assert(main_tree_node);

    main_tree_key = (box_main_tree_node_t *) main_tree_node->key;

    assert(main_tree_key);

    return main_tree_key->subtree;
}

static unsigned int get_sub_tree_max(rb_tree_node_t *main_tree_node)
{
    box_main_tree_node_t *main_tree_key = NULL;
    rb_tree_node_t *max_node = NULL;
    box_key_t *max_key = NULL;
    rb_tree_t *tree = NULL;

    main_tree_key = (box_main_tree_node_t *) main_tree_node->key;

    assert(main_tree_key);

    tree = main_tree_key->subtree;
    max_node = tree->max;

    /* We assume that there are nodes in the tree */
    assert(max_node);
    assert(max_node != &(tree->nil));

    max_key = (box_key_t *) max_node->key;

    assert(max_key);

    return max_key->val;
}

static unsigned int get_main_tree_node_val(rb_tree_node_t *main_tree_node)
{
    box_main_tree_node_t *main_tree_key = (box_main_tree_node_t *) main_tree_node->key;

    return main_tree_key->val;
}

static unsigned int get_sub_tree_node_val(rb_tree_node_t *sub_tree_node)
{
    box_key_t *sub_tree_key = (box_key_t *) sub_tree_node->key;

    return sub_tree_key->val;
}
