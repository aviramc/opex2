#ifndef __BOX_MENU_H__
#define __BOX_MENU_H__

#include <stdbool.h>

bool box_menu_insert(void *box_factory_t);
bool box_menu_remove(void *box_factory_t);
bool box_menu_get(void *box_factory_t);
bool box_menu_check(void *box_factory_t);

#endif /* __BOX_MENU_H__ */
