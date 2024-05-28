#ifndef GLOBALS_H
#define GLOBALS_H

#define GAME_HEIGHT 18
#define GAME_WIDTH 32

#define BOARD_HEIGHT 15
#define BOARD_WIDTH 30

#define TEXT_SIZE 255

#define SERVER_IP "::1"
#define MULTICAST_IP "ff02::1"
#define TCP_PORT 5551
#define SERVER_UDP_PORT 5556
#define SERVER_MULTIDIFF_PORT 5557
#define CLIENT_UDP_PORT_MIN 5558
#define CLIENT_UDP_PORT_MAX 5581

#define U 0
#define L 1
#define D 2
#define R 3
#define PB 4
#define RE 5

#define INFINITE -1

#define SIZE_MSG 100
#define MAX_CLIENTS 24
#define MAX_SERVERS 6

#define FFA 1
#define TDM 2

extern int server_id;
extern int udp_port;
extern int multidiff_port;

#endif // !GLOBALS_H
