#ifndef __MENU_H__
#define __MENU_H__

#include <stdbool.h>

typedef bool (*menu_callback_t)(void *);

typedef struct menu_item_s {
    menu_callback_t callback;
    char *description;
    void *arg;
} menu_item_t;

void print_menu(menu_item_t menu_items[], unsigned int items);
void run_menu(menu_item_t menu_items[], unsigned int items);

#endif /* __MENU_H__ */
