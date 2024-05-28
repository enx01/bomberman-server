#ifndef MENU_H
#define MENU_H

#include "../../headers/structs.h"

menu *setup_menu();

void render_menu(menu *, line *);
void compute_command(char *data);
void update_menu(menu *);

void run_menu(menu *, line *);

#endif // !MENU_H