#include <stdbool.h>
#include <stdio.h>

#include "box_factory.h"
#include "box_menu.h"

static void _get_dimensions(unsigned int *side, unsigned int *height)
{
    printf("Enter size of side: ");
    scanf("%u", side);
    printf("Enter size of height: ");
    scanf("%u", height);
}

bool box_menu_insert(void *box_factory_ptr)
{
    box_factory_t *factory = box_factory_ptr;
    unsigned int side = 0;
    unsigned int height = 0;

    _get_dimensions(&side, &height);

    if (!box_factory_insert(factory, side, height)) {
        printf("Fatal error: Insertion failed (out of memory)\n");
        return false;
    } else {
        printf("Inserted a box with side=%d and height=%d\n", side, height);
    }

    return true;
}

bool box_menu_remove(void *box_factory_ptr)
{
    box_factory_t *factory = box_factory_ptr;
    unsigned int side = 0;
    unsigned int height = 0;

    _get_dimensions(&side, &height);

    if (!box_factory_remove(factory, side, height)) {
        printf("Error: Box size not found\n");
    } else {
        printf("Removed a box with side=%d and height=%d\n", side, height);
    }

    return true;
}

bool box_menu_get(void *box_factory_ptr)
{
    box_factory_t *factory = box_factory_ptr;
    unsigned int side = 0;
    unsigned int height = 0;
    unsigned int found_side = 0;
    unsigned int found_height = 0;
    box_key_t *key = NULL;

    _get_dimensions(&side, &height);

    key = box_factory_get_box(factory, side, height, &found_side, &found_height);
    if (NULL == key) {
        printf("Error: No matching box found\n");
    } else {
        printf("Found a box with side=%d and height=%d\n", found_side, found_height);
    }

    return true;
}

bool box_menu_check(void *box_factory_ptr)
{
    box_factory_t *factory = box_factory_ptr;
    unsigned int side = 0;
    unsigned int height = 0;

    _get_dimensions(&side, &height);

    if (!box_factory_check_box(factory, side, height)) {
        printf("No matching box exists\n");
    } else {
        printf("A matching box exists\n");
    }

    return true;
}
