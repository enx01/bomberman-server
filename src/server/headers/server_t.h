#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "../../headers/structs.h"


void init_server(server_t *, int gamemode_flag);
void sv_add_player(server_t *, client_t *);
void sv_remove_player(server_t *, client_t *);

void sv_shutdown(server_t *);

#endif // !GAME_SERVER_H
