#include "headers/server_t.h"
#include "../headers/globals.h"
#include "../game/headers/game.h"
#include "headers/queue_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include "../headers/globals.h"
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../protocol/protocol_header.h"
#include "../protocol/messages/protocol_server_messages.h"


int get_unique_udp_port() {
    return udp_port++;
}

int get_unique_multidiff_port() {
    return multidiff_port++;
}

int create_new_game_sockets(server_t *s) {
    //*** Création de la socket server UDP
    s->udp_socket = socket(PF_INET6, SOCK_DGRAM, 0);
    if (s->udp_socket < 0) {
        perror("Erreur lors de la création de la socket server UDP");
        return -1;
    }

    //*** Configuration de l'adresse du server UDP
    memset(&s->udp_sockaddr, 0, sizeof(s->udp_sockaddr));
    s->udp_sockaddr.sin6_family = AF_INET6;
    s->udp_sockaddr.sin6_addr = in6addr_any;
    s->udp_sockaddr.sin6_port = htons(get_unique_udp_port());

    //*** Lier la socket UDP à l'adresse du server
    if (bind(s->udp_socket, (struct sockaddr *)&s->udp_sockaddr, sizeof(s->udp_sockaddr)) < 0) {
        perror("Erreur lors du bind de la socket server UDP");
        return -1;
    }

    s->multidiff_socket = socket(PF_INET6, SOCK_DGRAM, 0);
    if (s->multidiff_socket < 0) {
        perror("Erreur lors de la creation de la socket multicast du serveur");
        return -1;
    }

    memset(&s->multidiff_sockaddr, 0, sizeof(s->multidiff_sockaddr));
    s->multidiff_sockaddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, MULTICAST_IP, &s->multidiff_sockaddr.sin6_addr);
    s->multidiff_sockaddr.sin6_port = htons(get_unique_multidiff_port());

    int ifindex = if_nametoindex("eth0");
    if (setsockopt(s->multidiff_socket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) < 0) {
        perror("Erreur initialisation de l'intreface locale");
        return -1;
    }

    return 0;
}

void init_server(server_t *s, int gamemode_flag) {
    s->game_id = server_id++;
    s->players = malloc(sizeof(client_t) * 4);
    for (int i = 0; i < 4; i++) {
        s->players[i] = NULL;
    }

    s->players_count = 0;
    s->running = 0;
    s->gamemode = gamemode_flag;
    
    s->logic = setup_game();

    if (create_new_game_sockets(s) < 0) {
        fprintf(stderr, "Error creating game udp and multicast sockets");
    }

    printf("Initialized server_t with id : %d\n", s->game_id);
}

void sv_add_player(server_t *s, client_t *c) {

    for (int i = 0; i < 4; i++) {
        if (s->players[i] == NULL) {
            c->id = i;
            s->players[i] = c;
            s->players_count++;
        }
    }
    
    c->affected_sv_id = s->game_id;

    if (s->gamemode == TDM) {
        if (s->size_team_zero > s->size_team_one) {
            c->eq = 1;
            s->size_team_one++;
        }
        else {
            c->eq = 0;
            s->size_team_zero++;
        }
    }

}

void sv_remove_player(server_t *s, client_t *c) {
    for (int i = 0; i < s->players_count; i++) {
        if (s->players[i]->sock_fd == c->sock_fd) {
            s->players[i] = NULL;
            s->players_count--;
        }
    }
    
}

void sv_shutdown(server_t *s) {

    for (int i = 0; i < s->players_count; i++) {
        close(s->players[i]->sock_fd);
        s->players[i] = NULL;
    }
    free(s->players);

    s->running = 0;

    close(s->udp_socket);
    close(s->multidiff_socket);

    pthread_join(s->heart, NULL);
    
    s = NULL;
}