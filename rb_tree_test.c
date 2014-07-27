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

static void print_tree(rb_tree_t *tree)
{
    rb_tree_in_order(tree, tree->head, print_key);
    printf("\n");
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
    int keys[] = {73, 82, 76, 33, 64, 26, 29, 75, 11, 2};
    bool result = false;
    int *key = NULL;
    int *deleted = NULL;
    int temp = 0;
    unsigned int i = 0;
    const unsigned int unique_keys = sizeof(keys) / sizeof(int);

    tree = rb_tree_create(&compare_int);

    printf("Empty tree:\n");
    print_tree(tree);

    for (i = 0; i < unique_keys; i++) {
        printf("Inserting member %d:\n", keys[i]);
        assert(rb_tree_insert(tree, &keys[i], &result) == true);
        assert(result == false);
        print_tree(tree);
    }

    printf("\nRemoving the head of the tree (which is %d):\n", *(int *)tree->head->key);
    key = tree->head->key;
    assert(key == rb_tree_search_smallest(tree, key)->key);
    assert(rb_tree_remove(tree, key, (void **)&deleted));
    assert(deleted);
    print_tree(tree);
    
    printf("Inserting back %d:\n", *key);
    assert(rb_tree_insert(tree, key, &result) == true);
    assert(result == false);
    print_tree(tree);
    
    printf("Inserting again all 2nd keys\n");
    for (i = 0; i < unique_keys; i += 2) {
        printf("Inserting member %d:\n", keys[i]);
        assert(rb_tree_insert(tree, &keys[i], &result) == true);
        assert(result == true);
        print_tree(tree);
    }

    printf("Emptying tree:\n");
    for (i = 0; i < unique_keys; i++) {
        printf("Removing member %d", keys[i]);
        if (i % 2 == 0) {
            printf(" (removing twice)");
            assert(&keys[i] == rb_tree_search_smallest(tree, &keys[i])->key);
            assert(rb_tree_remove(tree, &keys[i], (void **)&deleted));
            assert(deleted == NULL);
        }
        printf(":\n");
        assert(&keys[i] == rb_tree_search_smallest(tree, &keys[i])->key);
        assert(rb_tree_remove(tree, &keys[i], (void **)&deleted));
        assert(deleted);
        print_tree(tree);
    }

    printf("Rebuilding tree...\n");
    for (i = 0; i < unique_keys; i++) {
        assert(rb_tree_insert(tree, &keys[i], &result) == true);
        assert(result == false);
    }

    printf("Verifying 'search_smallest'...\n");
    /* XXX: The following is hard coded based on the numbers in the test array (generated randomly) */
    temp = 65;
    assert(73 == *(int *)(rb_tree_search_smallest(tree, &temp)->key));
    temp = 73;
    assert(rb_tree_remove(tree, &temp, (void **)&deleted));
    assert(deleted);

    temp = 80;
    assert(82 == *(int *)(rb_tree_search_smallest(tree, &temp)->key));
    temp = 82;
    assert(rb_tree_remove(tree, &temp, (void **)&deleted));
    assert(deleted);

    temp = -30;
    assert(2 == *(int *)(rb_tree_search_smallest(tree, &temp)->key));
    temp = 2;
    assert(rb_tree_remove(tree, &temp, (void **)&deleted));
    assert(deleted);
    printf("Verify that keys 2, 73 & 82 don't exist in the tree\n");
    print_tree(tree);
    
    return 0;
}
