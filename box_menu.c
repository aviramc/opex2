#include <stdbool.h>
#include <stdio.h>

#include "box_menu.h"

static void _get_dimensions(unsigned int *side, unsigned int *height)
{
    printf("Enter size of side: ");
    scanf("%u", side);
    printf("Enter size of height: ");
    scanf("%u", height);
}
