#include <stdio.h>
#include <stdbool.h>

#include "box_factory.h"
#include "box_menu.h"
#include "menu.h"

int main(void)
{
    box_factory_t *factory = box_factory_create();
    menu_item_t menu_items[] = {{box_menu_insert, "Insert a box", factory},
                                {box_menu_remove, "Remove a box", factory},
                                {box_menu_get, "Get the sizes of an appropriate box", factory},
                                {box_menu_check, "Check if a box in an appropriate box exists", factory},
                                MENU_QUIT_ACTION,
    };

    if (NULL == factory) {
        printf("Fatal error: unable to create factory object (out of memory)\n");
        return -1;
    }

    menu_run(menu_items, sizeof(menu_items) / sizeof(menu_item_t));

    return 0;
}
