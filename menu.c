#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

void print_menu(menu_item_t menu_items[], unsigned int items)
{
    unsigned int i = 0;

    for (i = 0; i < items; ++i) {
        printf("%u. %s\n", i, menu_items[i].description);
    }
}


void run_menu(menu_item_t menu_items[], unsigned int items)
{
    unsigned int option;
    bool running = true;

    while (running) {
        /* TODO: Clear screen */
        print_menu(menu_items, items);
        /* TODO: Usage of scanf */
        scanf("%d", &option);
        if (option < items) {
            running = menu_items[option].callback(menu_items[option].arg);
        } else {
            printf("Invalid option: %d\n", option);
        }
    }
}
