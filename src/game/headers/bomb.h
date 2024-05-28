#ifndef BOMB_H
#define BOMB_H

#include "../../headers/structs.h"


bomb *create_bomb(int x, int y, int owner_id);
void explode_bomb(bomb *);
void update_bomb(bomb *);

#endif // !BOMB_H
