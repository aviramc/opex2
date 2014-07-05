#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "rb_tree.h"

static void print_key(rb_tree_t *tree, rb_tree_node_t *node)
{
    unsigned char color = '0';
    
    if (node->color == 0) {
        color = 'B';
    } else {
        color = 'R';
    }

    printf("%d #%u [%c] ", *((int *)node->key), node->count, color);
}

static int compare_int(void *a, void *b)
{
    int ia = *(int *)a;
    int ib = *(int *)b;

    if (ia < ib) {
        return -1;
    }

    if (ia == ib) {
        return 0;
    }

    return 1;
}

int main(void)
{
    rb_tree_t *tree = NULL;
    int keys[] = {2, 1, 3};
    bool result = false;
    int key = 0;
    unsigned int i = 0;

    tree = rb_tree_create(&compare_int);

    printf("Empty tree:\n");
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    printf("Inserting member...\n");
    rb_tree_insert(tree, &keys[0], &result);
    assert(result == false);
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    printf("Inserting two new members...\n");
    rb_tree_insert(tree, &keys[1], &result);
    assert(result == false);
    rb_tree_insert(tree, &keys[2], &result);
    assert(result == false);
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    printf("Removing member with key 1\n");
    key = 1;
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == true);
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    printf("Inserting member with key 1 twice\n");
    rb_tree_insert(tree, &keys[1], &result);
    assert(result == false);
    printf("First time\n");
    rb_tree_in_order(tree, tree->head, print_key);
    key = 1;
    rb_tree_insert(tree, &key, &result);
    assert(result == true);
    printf("\nSecond time\n");
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");
    
    printf("Inserting member with key 3 twice\n");
    rb_tree_insert(tree, &keys[2], &result);
    assert(result == true);
    printf("First time\n");
    rb_tree_in_order(tree, tree->head, print_key);
    key = 3;
    rb_tree_insert(tree, &key, &result);
    assert(result == true);
    printf("\nSecond time\n");
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    printf("Emptying tree\n");
    key = 2;
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == true);
    key = 1;
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == false);
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == true);
    key = 3;
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == false);
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == false);
    rb_tree_fetch_smallest(tree, &key, &result);
    assert(result == true);
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");

    return 0;
}
