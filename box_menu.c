#include <stdbool.h>
#include <stdio.h>
#include <math.h>

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
    printf("Requesting to inserted a box with side=%d and height=%d\n", side, height);

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
    printf("Requesting to removed a box with side=%d and height=%d\n", side, height);

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
    unsigned int found_side_square = 0;
    unsigned int found_height = 0;
    bool found = false;

    _get_dimensions(&side, &height);
    printf("Searching for a box with minimum side=%d and height=%d\n", side, height);

    found = box_factory_get_box(factory, side, height, &found_side_square, &found_height);
    if (found) {
        printf("Found a box with side=%d and height=%d\n", (unsigned int) sqrt((double) found_side_square), found_height);
    } else {
        printf("Error: No matching box found\n");
    }

    return true;
}

bool box_menu_check(void *box_factory_ptr)
{
    box_factory_t *factory = box_factory_ptr;
    unsigned int side = 0;
    unsigned int height = 0;

    _get_dimensions(&side, &height);
    printf("Checking if a box with minimum side=%d and height=%d exists\n", side, height);

    if (!box_factory_check_box(factory, side, height)) {
        printf("No matching box exists\n");
    } else {
        printf("A matching box exists\n");
    }

    return true;
}
