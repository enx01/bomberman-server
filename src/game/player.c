#include "headers/player.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

player *setup_player(int nb) {
    player *res = malloc(sizeof(player));
    if (!res) {
        res = NULL;
        return res;
    }
    memset(res, 0, sizeof(player));


    switch (nb)
    {
    case 0:
        res->pos_x = 0;
        res->pos_y = 0;
        break;
    case 1:
        res->pos_x = BOARD_WIDTH-1;
        res->pos_y = 0;
        break;
    case 2:
        res->pos_x = 0;
        res->pos_y = BOARD_HEIGHT-1;
        break;
    case 3:
        res->pos_x = BOARD_WIDTH-1;
        res->pos_y = BOARD_HEIGHT-1;
        break;
    default:
        res->pos_x = 1;
        res->pos_y = 1;
        break;

    }

    res->next_x = res->pos_x;
    res->next_y = res->pos_y;

    res->team_id = 0;

    update_player(res);
    
    res->has_bomb = 1;

    res->state = ALIVE;

    return res;
}

void player_handle_input(player *p, ACTION a) {
    switch(a) {
        case REVERT:
            break;
        case UP:
            p->next_y = p->pos_y - 1;
            break;
        case DOWN:
            p->next_y= p->pos_y + 1;
            break;
        case LEFT:
            p->next_x = p->pos_x - 1;
            break;
        case RIGHT:
            p->next_x = p->pos_x + 1;
            break;
        case SPACE:
            player_place_bomb(p);
            break;
        // case QUIT:
        //     curs_set(1); // Set the cursor to visible again
        //     endwin(); /* End curses mode */
        //     exit(EXIT_SUCCESS); // TODO : Handle user exiting game
        //     break;
    }
}

void player_place_bomb(player *p) {
    if (p->has_bomb) {
        p->is_placing_bomb = 1;
    }
}

void update_player(player *p) {
    p->pos_x = p->next_x;
    p->pos_y = p->next_y;

    if (p->state == DEAD) {
        p->pos_x = -1;
        p->pos_y = -1;
    }
} 

void reset_next(player *p) {
    p->next_x = p->pos_x;
    p->next_y = p->pos_y;
}
