/*
  box_factory.h - The main logic module of the exercise.
  This is the implementation of the box login operations.
 */

#include <stdbool.h>

#include "rb_tree.h"

#ifndef __BOX_FACTORY_H__
#define __BOX_FACTORY_H__

typedef struct box_key_s {
    unsigned int val;
} box_key_t;

typedef struct box_main_tree_node_s {
    unsigned int val;
    rb_tree_t *subtree;
} box_main_tree_node_t;

typedef struct box_factory_s {
    rb_tree_t *tree_by_side;   /* Tree by the key side */
    rb_tree_t *tree_by_height; /* Tree by the key height */
} box_factory_t;

/* box_factory_create - create an empty box factory.
   This will allocate and return the box factory structure.
   If an allocation error occurs, NULL is returned.
 */
box_factory_t* box_factory_create();

/* box_factory_insert - the exercise's BoxInsert.
   Returns false on errors (which can only happen due to an allocation error), otherwise true
 */
bool box_factory_insert(box_factory_t *factory, unsigned int side, unsigned int height);

/* box_factory_remove - the exercise's BoxRemove.
   Returns false if there's no box with the specified side & height, otherwise true.
 */
bool box_factory_remove(box_factory_t *factory, unsigned int side, unsigned int height);

/* box_factory_get_box - the exercise's GetBox.
   Returns NULL if there's no box that fits the given sizes.
 */
box_key_t* box_factory_get_box(box_factory_t *factory, unsigned int side, unsigned int height, unsigned int *found_side, unsigned int *found_height);

/* box_factory_check_box - the exercise's CheckBox.
   Returns true if a box exists, false otherwise.
 */
bool box_factory_check_box(box_factory_t *factory, unsigned int side, unsigned int height);

#endif /* __BOX_FACTORY_H__ */
