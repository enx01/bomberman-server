#include <stdlib.h>
#include "headers/bomb.h"
#include <stdio.h>

bomb *create_bomb(int x, int y, int o_id) {
    bomb *res = malloc(sizeof(bomb));
    res->pos_x = x;
    res->pos_y = y;
    res->state = BOMB;
    res->owner_id = o_id;
    res->time_remaining = 100; // game loop at 30 fps. 100 = 3s.
    return res;
}

void explode_bomb(bomb *b) {
    b->state = EXPLOSION;
    b->time_remaining = 33;
}

void update_bomb(bomb *b) {
    switch (b->state)
    {
    case BOMB:
        if (b->time_remaining == 0) {
            explode_bomb(b);
        }
        else {
            --b->time_remaining;
        }
        break;
    case EXPLOSION:
        if (b->time_remaining == 0) {
            b->state = REMOVE;
        }
        else {
            --b->time_remaining;
        }
        break;
    case REMOVE:
        break;
    default:
        break;
    }
}

