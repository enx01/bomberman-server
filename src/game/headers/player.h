#ifndef PLAYER_H
#define PLAYER_H
#include "../../headers/structs.h"

player *setup_player();
player *setup_player(int);
void player_handle_input(player *p, ACTION);
void update_player(player *p);
void reset_next(player *p);
void player_place_bomb(player *p);


#endif // !PLAYER_H
