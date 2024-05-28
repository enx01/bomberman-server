#include "headers/menu.h"
#include <ncurses.h>
#include <stdlib.h>

menu *setup_menu()
{
    menu *res = malloc(sizeof(menu));

    res->state = TITLE;

    return res;
}

void render_menu(menu *m, line *l) {

    // Borders
    for (int x = 0; x < GAME_WIDTH; x++) {
        mvaddch(0, x, '-');
        mvaddch(GAME_HEIGHT-2, x, '-');
    }
    for (int y = 0; y < GAME_HEIGHT-1; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, GAME_WIDTH-1, '|');
    }
    mvaddch(0, 0,'o');
    mvaddch(0, GAME_WIDTH-1,'o');
    mvaddch(GAME_HEIGHT-2, 0,'o');
    mvaddch(GAME_HEIGHT-2, GAME_WIDTH-1,'o');   

    switch (m->state)
    {
    case TITLE:
        
        mvprintw(4, 10, "bomberman !");
        mvprintw(9, 1, "`ffa` - play free-for-all");
        mvprintw(10, 1, "`tdm` - play team deathmatch");

        
        break;
    case LOBBY:
        
        mvprintw(4, 10, "en attente de joueur");

        break;
    default:
        break;
    }

    
    refresh(); // Apply the changes to the terminal
}

void run_menu(menu *m, line *l) {
    render_menu(m,l);
}