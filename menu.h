#ifndef __MENU_H__
#define __MENU_H__

#include <stdbool.h>

#define MENU_QUIT_ACTION {menu_quit, "Quit", NULL}

typedef bool (*menu_callback_t)(void *);

typedef struct menu_item_s {
    menu_callback_t callback;
    char *description;
    void *arg;
} menu_item_t;

void menu_print(menu_item_t menu_items[], unsigned int items);
void menu_run(menu_item_t menu_items[], unsigned int items);
bool menu_quit(void *nothing);

#endif /* __MENU_H__ */
