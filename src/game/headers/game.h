#ifndef GAME_H
#define GAME_H
#include "../../headers/structs.h"


game *setup_game();
void update_game(game *, uint8_t *, uint8_t *, uint8_t *);
void render_game(game *, line *l);
void run_game(game *, line *l);

int game_add_player(game *, player *); // Return 1 if successful 0 if game full;

void free_game(game *g);

#endif // !GAME_H
