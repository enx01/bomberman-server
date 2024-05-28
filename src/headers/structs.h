#ifndef STRUCTS_H
#define STRUCTS_H

#include "globals.h"
#include <stdint.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <pthread.h>


typedef enum ACTION { UP, LEFT, DOWN, RIGHT, SPACE, REVERT, QUIT, ENTER } ACTION;
typedef enum BOMBSTATE { BOMB, EXPLOSION, REMOVE } BOMBSTATE;
typedef enum PLAYERSTATE { ALIVE, DEAD } PLAYERSTATE;
typedef enum MENUSTATE { TITLE, LOBBY, IN_GAME } MENUSTATE;
typedef enum CONNECTIONSTATE { NO_CONNECTION, ESTABLISHED } CONNECTIONSTATE;
typedef enum GAME_SERVERSTATE { WAITING_FOR_PLAYERS, IN_PROGRESS, GAME_OVER } GAME_SERVERSTATE;


struct bomb {
    int pos_x, pos_y,
        owner_id,
        time_remaining;
    BOMBSTATE state;
};
typedef struct bomb bomb;

struct player {
    int pos_x, pos_y, next_x, next_y,
        id,
        team_id,
        has_bomb, is_placing_bomb;
        PLAYERSTATE state;
};
typedef struct player player;


struct board {
    char *grid; //[(GAME_WIDTH - 3) * (GAME_HEIGHT - 2)]
    int w;
    int h;
};
typedef struct board board;

struct game {
    int w,h;
    int player_count;
    int ongoing;
    player *players_list[4];
    bomb *bomb_list[4];
    board *b;
};
typedef struct game game;

struct line {
    char data[TEXT_SIZE];
    int cursor;
};

typedef struct line line;

struct menu {
    MENUSTATE state;
};
typedef struct menu menu;



// Network

struct client_t{
    int sock_fd;
    uint8_t id;
    uint8_t eq;
    struct sockaddr_in6 address;
    uint16_t port_udp;
    uint16_t port_multidiff;
    uint8_t addr_multidiff[16];
    int ready, affected_sv_id, nb_sent;
    player *proxy;
};
typedef struct client_t client_t;


struct server_t{
    int game_id;
    int running;
    int players_count;
    int gamemode;

    int size_team_zero,
        size_team_one;

    pthread_t heart;

    int udp_socket, multidiff_socket;
    struct sockaddr_in6 udp_sockaddr, multidiff_sockaddr;

    client_t **players;
    game *logic;

};
typedef struct server_t server_t;

struct queue_t {
    client_t **cl;
    int cl_count;
    server_t **sv;
    int sv_count;
};
typedef struct queue_t queue_t;

#endif // !STRUCTS_H