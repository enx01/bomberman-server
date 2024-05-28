#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <net/if.h>
#include "protocol/protocol_header.h"
#include "protocol/messages/protocol_server_messages.h"
#include "protocol/messages/protocol_client_messages.h"
#include "headers/globals.h"
#include "headers/structs.h"
#include "game/headers/player.h"
#include "game/headers/game.h"
#include "server/headers/client_t.h"
#include "server/headers/server_t.h"
#include "server/headers/queue_handler.h"
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>


int nanosleep(const struct timespec *req, struct timespec *rem);

// Variables globales
int tcp_server_socket;

int freq = 200; // ms

// Client_handler
struct pollfd clients[MAX_CLIENTS];
int poll_i, poll_maxi, nready;

queue_t queue;

struct sockaddr_in6 tcp_serv_addr;

pthread_t client_handler;
// pthread_t sv_handlers[MAX_SERVERS];

pthread_mutex_t servers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int create_tcp_socket() {
    //*** Création de la socket server pour TCP
    tcp_server_socket = socket(PF_INET6, SOCK_STREAM, 0);
    if (tcp_server_socket < 0) {
        perror("Erreur lors de la création de la socket server TCP");
        return 1;
    }

    //*** Configuration de l'adresse du server TCP
    memset(&tcp_serv_addr, 0, sizeof(tcp_serv_addr));
    tcp_serv_addr.sin6_family = AF_INET6;
    tcp_serv_addr.sin6_addr = in6addr_any;
    tcp_serv_addr.sin6_port = htons(TCP_PORT);

    //*** Lier la socket TCP à l'adresse du server
    if (bind(tcp_server_socket, (struct sockaddr *)&tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) {
        perror("Erreur lors du bind de la socket serveur TCP");
        return 1;
    }

    if (fcntl(tcp_server_socket, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl");
        return 1;
    }

    //*** Mettre en écoute la socket TCP
    if (listen(tcp_server_socket, 5) < 0) {
        perror("Erreur lors de la mise en écoute de la sokcet server TCP");
        return 1;
    }

    return 0;
}

int find_last_msg(action_data_msg *msgs, int nb_sent) {
    int max = 0;
    int res = 0;
    int nb_zero = 0;

    for (int i = 0; i < nb_sent; i++) {
        uint16_t decoded_codereq;
        uint8_t decoded_id;
        uint8_t decoded_eq;
        uint16_t num;
        uint8_t action;

        decode_action_data_msg(&msgs[i], &decoded_codereq, &decoded_id, &decoded_eq, &num, &action);

        if (num == 0) {
            nb_zero++;
            if (nb_zero > 1) {
               return action; 
            }
        }
        if (num > max) { 
            max = num;
            res = action;
        }
    }

    return res;
}

void *sv_job(void *args) {
    server_t *sv = (server_t *) args;
    printf("SV-%d thread started !\n", sv->game_id);
    sv->running = 1;

    int elapsed_intervals = 0;

    struct timespec r;


    r.tv_sec = freq / 1000;
    r.tv_nsec = (freq % 1000) * 1000000;

    struct pollfd players_fd[4];
    int max = 0;

    memset(clients, 0, sizeof(clients));

    int nb_ready = 0, last_nb_ready = 0;
    // TODO : Recieve ready message from each players
    while (nb_ready != 4) {
        nb_ready = 0;

        for (int i = 0; i < sv->players_count; i++) {
            if (sv->players[i]->sock_fd == 0) {
                sv->players_count--;
            }

            if (sv->players[i]->ready) {
                nb_ready++;
                players_fd[i].fd = sv->players[i]->sock_fd;
                players_fd[i].events = POLLIN | POLLOUT;
                max++;
            }
        }

        if (nb_ready != last_nb_ready) {
            printf("Ready : %d\n", nb_ready);
            last_nb_ready = nb_ready;
        }

        if (sv->players_count == 0) {
            sv_shutdown(sv);
        }

        sleep(1);
    }

    for (int i = 0; i < sv->players_count; i++) {
        pthread_mutex_lock(&clients_mutex);
        sv->players[i]->proxy = setup_player(i);
        game_add_player(sv->logic, sv->players[i]->proxy);
        pthread_mutex_unlock(&clients_mutex);
    }

    struct sockaddr_in6 client_addr;
    socklen_t addr_len = sizeof(client_addr);

    uint16_t broadcast_messages_id = 0;
    uint16_t freq_messages_id = 0;

    uint8_t whole_tiles[BOARD_HEIGHT * BOARD_WIDTH];
    uint8_t diff[BOARD_HEIGHT * BOARD_WIDTH];
    uint8_t nb_diff;

    int a = 0;

    action_data_msg msgs_clients[4][8191*2];
    server_chat_msg to_transmit[100];
    int nb_outgoing_chat = 0;

    int end_game = -1;

    struct sockaddr_in6 author;
    socklen_t len;


    printf("game logic state : players_count : %d\n", sv->logic->player_count);

    while(sv->running) {
        poll(players_fd, max, INFINITE);

        for (int i = 0; i < max; i++) {
            
            if (players_fd[i].revents | (POLLIN | POLLHUP | POLLERR)) {

                if (players_fd[i].revents & POLLHUP) {
                    printf("SV-%d : Socket %d hung up\n", sv->game_id, players_fd[i].fd);
                    sv_shutdown(sv);
                }
                else if (players_fd[i].revents & POLLERR) {
                    printf("SV-%d : Error socket %d\n", sv->game_id, players_fd[i].fd);
                    sv_shutdown(sv);
                }
                else { // Case POLLIN, le client est pret a etre ecoute.
                    client_chat_msg incomming;
                    memset(&incomming, 0, sizeof(incomming));

                    size_t length = sizeof(incomming);
                    size_t total_received = 0;
                    ssize_t bytes_received;

                    while (total_received < length) {
                        bytes_received = recv(players_fd[i].fd, (char *)&incomming + total_received, length - total_received, 0);

                        total_received += bytes_received;
                    }
                
                    // int n = recv(temp_socket, &incomming, sizeof(incomming), 0);
                    if (bytes_received < 0) {
                        fprintf(stderr, "SV-%d : Error! Couldn't read data from : %d!\n", sv->game_id, players_fd[i].fd);
                        sv_shutdown(sv);
                    }
                    else if (bytes_received == 0) {
                        fprintf(stderr, "SV-%d : Error! Client disconnected! : %d!\n", sv->game_id, players_fd[i].fd);
                        sv_shutdown(sv);
                    }
                    else {
                        uint16_t decoded_codereq;
                        uint8_t decoded_id;
                        uint8_t decoded_eq;

                        encode_protocol_header(&to_transmit[nb_outgoing_chat].header, decoded_codereq, decoded_id, decoded_eq);
                        memcpy(to_transmit[nb_outgoing_chat].data, incomming.data, sizeof(incomming.data));

                        nb_outgoing_chat++;
                    }
                }

            }
            else if (players_fd[i].revents | POLLOUT) {
                if (nb_outgoing_chat != 0) {

                }
                else if (end_game != -1) {
                    end_game_msg egm;
                    memset(&egm, 0, sizeof(egm));

                    uint16_t codereq = sv->gamemode ? 15 : 16;
                    uint8_t id;
                    uint8_t eq;


                    if (sv->gamemode == FFA) {
                        id = end_game;
                        eq = 0;
                    }
                    else {
                        id = end_game;
                        eq = 0;
                    }

                    encode_protocol_header(&egm.header, codereq, id, eq);


                    int sent = sendto(players_fd[i].fd, &egm, sizeof(egm), MSG_NOSIGNAL, (struct sockaddr *)&sv->players[i]->address, (socklen_t)sizeof(sv->players[i]->address));
                    if (sent != sizeof(egm)) {
                        fprintf(stderr, "SV-%d : Error sending end game message.\n");
                    }
                    else {
                        printf("SV-%d : End game message successfully sent to client %d\n", players_fd[i].fd);
                        players_fd[i].fd = -1;
                        max--;

                        if (max == 0) {
                            sv->running = 0;
                        }
                    }
                }
            }
        }
        

        action_data_msg incomming;
        memset(&incomming, 0, sizeof(incomming));

        size_t length = sizeof(incomming);

        size_t total_received = 0;
        ssize_t bytes_received;

        while (total_received < length) {
            bytes_received = recv(sv->udp_socket, (char *)&incomming + total_received, length - total_received, 0);
            total_received += bytes_received;
        }

        // int n = recv(temp_socket, &incomming, sizeof(incomming), 0);
        if (bytes_received < 0) {
            fprintf(stderr, "SV-%d : Error! Couldn't read data from player %d!\n", sv->game_id);           

            sv_shutdown(sv);
        }
        else if (bytes_received == 0) {
            fprintf(stderr, "SV-%d : Error! Client disconnected!\n", sv->game_id);
            sv_shutdown(sv);
        }
        else {
            uint16_t decoded_codereq;
            uint8_t decoded_id;
            uint8_t decoded_eq;
            uint16_t num;
            uint8_t action;

            decode_action_data_msg(&incomming, &decoded_codereq, &decoded_id, &decoded_eq, &num, &action);

            msgs_clients[decoded_id][sv->players[decoded_id]->nb_sent++] = incomming;

        }

        for (int i = 0; i < sv->players_count; i++) {
            ACTION ac = find_last_msg(msgs_clients[i], sv->players[i]->nb_sent);
            player_handle_input(sv->players[i]->proxy, ac);
        }
        

        uint8_t whole_tiles[BOARD_HEIGHT * BOARD_WIDTH];
        uint8_t diff[BOARD_HEIGHT * BOARD_WIDTH];
        uint8_t nb_diff;


        update_game(sv->logic, diff, &nb_diff, whole_tiles);

        if (sv->logic->ongoing != 1) {
            end_game = 1;
            continue;
        }       
    
        game_grid_msg diff_msg;

        encode_protocol_header(&diff_msg.header, 12, 0, 0);
        diff_msg.num = freq_messages_id;
        freq_messages_id += freq_messages_id % 2^16;

        int s = sendto(sv->multidiff_socket, &diff_msg, sizeof(diff_msg), 0,
                    (struct sockaddr*)&sv->multidiff_sockaddr, sizeof(sv->multidiff_sockaddr));
        if (s < 0) {
            fprintf(stderr, "SV-%d : Couldn't send diff grid message ! :-(\n", sv->game_id);
        }



        if (elapsed_intervals == 5) { // On a effectue 5 boucles a 200ms, soit 1000ms se sont ecoules.
            game_grid_msg whole_board;

            encode_protocol_header(&whole_board.header, 11, 0, 0);
            whole_board.num = broadcast_messages_id;
            broadcast_messages_id += broadcast_messages_id % 2^16;


            int s = sendto(sv->multidiff_socket, &whole_board, sizeof(whole_board), 0,
                (struct sockaddr*)&sv->multidiff_sockaddr, sizeof(sv->multidiff_sockaddr));
            if (s < 0) {
                fprintf(stderr, "SV-%d : Couldn't send whole grid message ! :-(\n", sv->game_id);
            }
        }



        elapsed_intervals++;
        nanosleep(&r, NULL);
    }
}

void *client_handler_job() {
    struct sockaddr_in6 client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;
    int temp_socket;

    printf("~CLIENT-HANDLER thread running..~\n");


    while (1) {
        nready = poll(clients, poll_maxi+1, INFINITE);

        if (nready < 0) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        // Verification de nouvelles connections
        if (clients[0].revents & POLLIN) {
            if ( (client_socket = accept(tcp_server_socket, (struct sockaddr *) &client_addr, &client_addr_len)) < 0 ) {
                perror("accept");
                exit(EXIT_FAILURE);
            }


            char client_ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(client_addr.sin6_addr), client_ip, INET6_ADDRSTRLEN);
            printf("CLIENT-HANDLER : Accepted socket %d (%s : %hu)\n",client_socket, client_ip, ntohs(client_addr.sin6_port));

            

            // On enregistre le socket du client
            for (poll_i = 1; poll_i < MAX_CLIENTS; poll_i++) {
                if (clients[poll_i].fd < 0) {
                    clients[poll_i].fd = client_socket;
                    clients[poll_i].events = POLLIN;

                    break;
                }
            }

            if (poll_i == MAX_CLIENTS) {
                fprintf(stderr, "CLIENT-HANDLER : Error! Too many clients\n");
                close(client_socket);
                clients[poll_i].fd = -1;
            }
            else {
                client_t *new_cl = malloc(sizeof(client_t));

                new_cl->sock_fd = client_socket;
                new_cl->address = client_addr;

                add_client(&clients_mutex, new_cl, &queue);

            }

            

            if (poll_i > poll_maxi) poll_maxi = poll_i;

            if (--nready <= 0) continue;

        }

        // Verification de donnees entrantes
        for (poll_i = 1; poll_i <= poll_maxi; poll_i++) {
            if ( (temp_socket = clients[poll_i].fd) < 0) continue;

            if (clients[poll_i].revents & (POLLIN | POLLHUP | POLLERR)) {
                if (clients[poll_i].revents & POLLHUP) {
                    printf("CLIENT-HANDLER : Socket %d hung up\n", temp_socket);
                    goto close_and_free;
                }
                if (clients[poll_i].revents & POLLERR) {
                    printf("CLIENT-HANDLER : Error! socket %d\n", temp_socket);
                    goto close_and_free;
                }

                protocol_header incomming;
                memset(&incomming, 0, sizeof(incomming));

                size_t length = sizeof(incomming);

                size_t total_received = 0;
                ssize_t bytes_received;

                 while (total_received < length) {
                    bytes_received = recv(temp_socket, (char *)&incomming + total_received, length - total_received, 0);
                    
                    total_received += bytes_received;
                }
                
                // int n = recv(temp_socket, &incomming, sizeof(incomming), 0);
                if (bytes_received < 0) {
                    fprintf(stderr, "CLIENT-HANDLER : Error! Couldn't read data from : %d!\n", temp_socket);
                    goto close_and_free;
                }
                else if (bytes_received == 0) {
                    printf("CLIENT-HANDLER : Goodbye, %d!\n", temp_socket);
                    goto close_and_free;
                }
                else {
                    printf("CLIENT-HANDLER : Recieved message from : %d!\n", temp_socket);
                    
                    uint16_t decoded_codereq = 0;
                    uint8_t decoded_id = 0;
                    uint8_t decoded_eq = 0;

                    decode_protocol_header(&incomming, &decoded_codereq, &decoded_id, &decoded_eq);

                    printf("CLIENT-HANDLER : Decoded message : \ncr = %d id = %d eq = %d\n", decoded_codereq, decoded_id, decoded_eq);

                    if (decoded_codereq <= 0) {
                        fprintf(stderr, "CLIENT-HANDLER : Error! Recieved codereq : %d, disconnecting client.\n", decoded_codereq);
                        goto close_and_free;
                    }
                    
                    server_t *to_join = NULL;
                    client_t *c = find_client(&clients_mutex, &queue, temp_socket);

                    switch (decoded_codereq)
                    {
                    case FFA:
                        // Join game FFA
                        to_join = find_server(&servers_mutex, FFA, &queue);
                        printf("found server : %d\n", to_join->game_id);
                        break;
                    case TDM:
                        // Join game TDM
                        to_join = find_server(&servers_mutex, TDM, &queue);
                        printf("found server : %d\n", to_join->game_id);
                        break;     
                    case 3 :
                        pthread_mutex_lock(&clients_mutex);
                        c->ready = 1;
                        pthread_mutex_unlock(&clients_mutex);

                        clients[poll_i].fd = -1;
                        break;     
                    case 4 :
                        pthread_mutex_lock(&clients_mutex);
                        c->ready = 1;
                        pthread_mutex_unlock(&clients_mutex);

                        clients[poll_i].fd = -1;
                        break;
                    case 7:
                        // Send global message
                        break;
                    case 8:
                        // Send team message
                        break;          
                    default:
                        fprintf(stderr, "CLIENT-HANDLER : Error! Recieved codereq : %d, disconnecting client.\n", decoded_codereq);
                        goto close_and_free;
                    }


                    if (to_join != NULL) {
                        printf("adding client to server\n");

                        if (c == NULL) {
                            fprintf(stderr, "CLIENT-HANDLER : Error! Client : %d can't be found.\n", temp_socket);
                            goto close_and_free;
                        }

                        pthread_mutex_lock(&clients_mutex);
                        pthread_mutex_lock(&servers_mutex);
                        sv_add_player(to_join, c);
                        pthread_mutex_unlock(&servers_mutex);
                        pthread_mutex_unlock(&clients_mutex);
                        
                        clients[poll_i].events = POLLOUT;

                        // Un joueur a ete ajoute a une partie, il faut alors verifier si le thread correspondant est lance ou s'il faut le demarrer
                        pthread_mutex_lock(&servers_mutex);
                        if (!to_join->running) {
                            pthread_create(&to_join->heart, NULL, sv_job, to_join);
                        }
                        pthread_mutex_unlock(&servers_mutex);
                    }

                    
                    continue;
                }
                

                close_and_free :
                    printf("CLIENT-HANDLER : Closing socket %d\n", temp_socket);
                    client_t *c = find_client(&clients_mutex, &queue, temp_socket);
                    server_t *s = sv_get(&servers_mutex, &queue, c->affected_sv_id);

                    sv_remove_player(s, c);

                    close(temp_socket);
                    clients[poll_i].fd = -1;
                    continue;
            }
            if (clients[poll_i].revents & POLLOUT) { // Client pret a l'ecoute
                client_t *c = find_client(&clients_mutex, &queue, temp_socket);

                if (!c->ready) {
                    // Envoi du message de confirmation au client
                    server_t *s = sv_get(&servers_mutex, &queue, c->affected_sv_id);

                    init_game_msg igm;
                    memset(&igm, 0, sizeof(igm));

                    uint16_t codereq = s->gamemode ? 9 : 10;
                    encode_protocol_header(&igm.header, codereq, c->id, c->eq);
                    igm.portudp = s->udp_sockaddr.sin6_port; // Deja en big endian
                    igm.portmdiff = s->multidiff_sockaddr.sin6_port; // Idem
                    snprintf(igm.adrmdiff, sizeof(MULTICAST_IP), "%s", MULTICAST_IP);

                    int sent = sendto(c->sock_fd, &igm, sizeof(igm), MSG_NOSIGNAL, (struct sockaddr *)&c->address, (socklen_t)sizeof(c->address));
                    if (sent != sizeof(igm)) {
                        if (errno == EPIPE) {
                            fprintf(stderr, "CLIENT-HANDLER : EPIPE error on socket %d\n", temp_socket);
                            goto close_and_free;
                        } else {
                            perror("send");
                        }
                    }
                    else {
                        printf("CLIENT-HANDLER : Init game message successfully sent to client %d\n", c->sock_fd);
                        clients[poll_i].events = POLLIN;
                    }
                }
                else {
                    // TODO Handle chat
                }
            }
        }

        sleep(1);
    }
    
}


// Initialisation des variables utilisees par le client_handler
void client_handler_init() {
    printf("~Starting CLIENT-HANDLER thread..~\n");

    memset(clients, 0, sizeof(clients));

    clients[0].fd = tcp_server_socket;
    clients[0].events = POLLIN;

    for (poll_i = 1; poll_i < MAX_CLIENTS; poll_i++) {
        clients[poll_i].fd = -1;
    }

    poll_maxi = 0;

    init_queue(&queue);

    pthread_create(&client_handler, NULL, client_handler_job, NULL);
}



int main() {
    
    printf("~Booting up server~\n");
    int tcp_socket_created = create_tcp_socket();
    if (tcp_socket_created == 1) return EXIT_FAILURE;
    printf("- TCP socket created!\n");

    signal(SIGPIPE, SIG_IGN);

    client_handler_init();

    pthread_join(client_handler, NULL);

    close(tcp_server_socket);

    return EXIT_SUCCESS;
}
