#include <stdio.h>
#include <stdbool.h>

#include "menu.h"

#define NUMBER_OF_ACTIONS (5)

static bool test_callback(void *nothing)
{
    printf("AAAA\n");
    return true;
}

static bool exit_callback(void *nothing)
{
    printf("Goodbye\n");
    return false;
}

int main(void)
{
    menu_item_t MENU_ITEMS[NUMBER_OF_ACTIONS] = {{test_callback, "Test callback", NULL},
                                                 {exit_callback, "Exit", NULL},
    };

    run_menu(MENU_ITEMS, 2);
    return 0;
}
