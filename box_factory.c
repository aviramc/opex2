#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "rb_tree.h"
#include "box_factory.h"

/* create_key - key creation function for the trees. The key would be the same, but the comparison
   function would be different. Returns NULL on an allocation failure.
 */
static box_key_t* create_key(unsigned int side, unsigned int height);

/* compare_keys_vsh, compare_keys_vhs - key comparison functions for the trees.
 */
static int compare_keys_vsh(void *a, void *b);
static int compare_keys_vhs(void *a, void *b);

/* Get the volume of a given box size */
static unsigned int get_volume(box_key_t *key);

box_factory_t* box_factory_create()
{
    box_factory_t *factory = NULL;
    rb_tree_t *rb_tree = NULL;

    factory = calloc(sizeof(box_factory_t), 1);
    if (NULL == factory) {
        return NULL;
    }

    rb_tree = rb_tree_create((rb_tree_key_cmp_t) compare_keys_vsh);
    if (NULL == rb_tree) {
        free(factory);
        return NULL;
    }
    factory->tree_by_vsh = rb_tree;

    rb_tree = rb_tree_create((rb_tree_key_cmp_t) compare_keys_vhs);
    if (NULL == rb_tree) {
        free(factory->tree_by_vsh);
        free(factory);
        return NULL;
    }
    factory->tree_by_vhs = rb_tree;

    return factory;
}

bool box_factory_insert(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_key_t **deleted = NULL;
    box_key_t *key = NULL;
    bool exists_in_vsh = false;
    bool exists_in_vhs = false;

    /* First try to create the two keys, so that if allocation errors occur, no tree would
       be changed */
    key = create_key(side, height);
    if (NULL == key) {
        return false;
    }

    if (false == rb_tree_insert(factory->tree_by_vsh, key, &exists_in_vsh)) {
        free(key);
        return false;
    }

    if (false == rb_tree_insert(factory->tree_by_vhs, key, &exists_in_vhs)) {
        /* Remove the key from the vsh tree. Note that the key must be there, thus the assert */
        assert(true == rb_tree_remove(factory->tree_by_vsh, key, (void **)&deleted));
        /* If a key has been deleted, it must be the one we've just inserted. */
        assert((NULL == deleted) || (*deleted == key));
        free(key);
        return false;
    }

    /* Verify that the two trees contain the same data */
    assert(exists_in_vsh == exists_in_vhs);

    /* No need to hold the key if it is not stored in any of the trees */
    if (exists_in_vsh) {
        free(key);
    }

    return true;
}

bool box_factory_remove(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_key_t *key = NULL;
    box_key_t *deleted_in_vsh = NULL;
    box_key_t *deleted_in_vhs = NULL;
    bool result = false;

    key = create_key(side, height);
    if (NULL == key) {
        return false;
    }

    result = rb_tree_remove(factory->tree_by_vsh, key, (void **)&deleted_in_vsh);
    /* See if the key exists in the tree(s) */
    if (result == false) {
        /* Note that the two trees contain the same data, so no need to search in the other tree */
        free(key);
        return false;
    }

    result = rb_tree_remove(factory->tree_by_vhs, key, (void **)&deleted_in_vhs);
    /* The key existed already, so it must exist now... */
    assert(result);
    /* We use the same memory of keys for both trees */
    assert(deleted_in_vsh == deleted_in_vhs);

    if (deleted_in_vsh) {
        free(deleted_in_vsh);
    }

    free(key);

    return true;
}

box_key_t * box_factory_get_box(box_factory_t *factory, unsigned int side, unsigned int height)
{
    box_key_t *key_by_side = NULL;
    box_key_t *key_by_height = NULL;
    box_key_t *key = NULL;

    key = create_key(side, height);
    if (NULL == key) {
        return NULL;
    }

    key_by_side = rb_tree_search_smallest(factory->tree_by_vsh, key);
    key_by_height = rb_tree_search_smallest(factory->tree_by_vhs, key);

    /* If the keys found are equal, there are three options:
         1. There's only one box size that matches.
         2. There's no box size size that matches.
         3. The exact box size was found.
     */
    if (key_by_side == key_by_height) {
        free(key);
        return key_by_side;
    }

    /* If the keys found are different, return the one with the smaller volume. */
    if (get_volume(key_by_side) < get_volume(key_by_height)) {
        free(key);
        return key_by_side;
    }

    free(key);
    return key_by_height;
}

/* TODO: Implement! */
bool box_factory_check_box(box_factory_t *factory, unsigned int side, unsigned int height)
{
    return true;
}

static box_key_t* create_key(unsigned int side, unsigned int height)
{
    box_key_t *key = calloc(sizeof(box_key_t), 1);

    if (NULL == key) {
        return NULL;
    }

    key->side = side;
    key->height = height;

    return key;
}

static int compare_keys_vsh(void *a, void *b)
{
    box_key_t *key_a = a;
    box_key_t *key_b = b;
    unsigned int volume_a = get_volume(key_a);
    unsigned int volume_b = get_volume(key_b);

    /* Compare volumes */
    if (volume_a < volume_b) {
        return -1;
    }

    if (volume_a > volume_b) {
        return 1;
    }

    /* volume_a == volume_b, so compare sides */
    if (key_a->side < key_b->side) {
        return -1;
    }

    if (key_a->side > key_b->side) {
        return 1;
    }

    /* sides are equal, so compare heights */
    if (key_a->height < key_b->height) {
        return -1;
    }

    if (key_a->height > key_b->height) {
        return 1;
    }

    /* All is equal */
    return 0;
}

static int compare_keys_vhs(void *a, void *b)
{
    box_key_t *key_a = a;
    box_key_t *key_b = b;
    unsigned int volume_a = get_volume(key_a);
    unsigned int volume_b = get_volume(key_b);

    /* Compare volumes */
    if (volume_a < volume_b) {
        return -1;
    }

    if (volume_a > volume_b) {
        return 1;
    }

    /* volume_a == volume_b, so compare heights */
    if (key_a->height < key_b->height) {
        return -1;
    }

    if (key_a->height > key_b->height) {
        return 1;
    }

    /* heights are equal, so compare sides */
    if (key_a->side < key_b->side) {
        return -1;
    }

    if (key_a->side > key_b->side) {
        return 1;
    }

    /* All is equal */
    return 0;
}

/* Get the volume of a given box size */
static unsigned int get_volume(box_key_t *key)
{
    return key->side * key->side * key->height;
}
