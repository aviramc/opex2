#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

void menu_print(menu_item_t menu_items[], unsigned int items)
{
    unsigned int i = 0;

    for (i = 0; i < items; ++i) {
        printf("%u. %s\n", i, menu_items[i].description);
    }
}


void menu_run(menu_item_t menu_items[], unsigned int items)
{
    unsigned int option;
    bool running = true;

    while (running) {
        menu_print(menu_items, items);
        scanf("%d", &option);
        if (option < items) {
            running = menu_items[option].callback(menu_items[option].arg);
        } else {
            printf("Invalid option: %d\n", option);
        }
        printf("\n");
    }
}

bool menu_quit(void *nothing)
{
    return false;
}
