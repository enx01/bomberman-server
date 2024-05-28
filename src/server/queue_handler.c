#include "headers/queue_handler.h"
#include "headers/server_t.h"
#include <stdlib.h>
#include <stdio.h>

void init_queue(queue_t *q) {
    q->cl = malloc(sizeof(client_t) * MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        q->cl[i] = NULL;
    }
    
    q->sv = malloc(sizeof(server_t) * MAX_SERVERS);
    for (int i = 0; i < MAX_SERVERS; i++) {
        q->sv[i] = NULL;
    }
}

void add_client(pthread_mutex_t *p, client_t *client, queue_t *q) {
    pthread_mutex_lock(p);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (q->cl[i] == NULL) {   
            q->cl[i] = client;
            pthread_mutex_unlock(p);
            break;
        }
    } 

    pthread_mutex_unlock(p);
}


// N'appeller qu'apres avoir deconnecte le client !!
void remove_client(pthread_mutex_t *p, client_t *client, queue_t *q) {
    pthread_mutex_lock(p);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (q->cl[i] != NULL) { 
            if (q->cl[i]->sock_fd == client->sock_fd) {
                free(q->cl[i]);
                q->cl[i] = NULL;
            }
        }
    } 

    pthread_mutex_unlock(p);
}

void add_server(pthread_mutex_t *p, server_t *server, queue_t *q) {
    pthread_mutex_lock(p);

    printf("adding server to queue..\n");

    for (int i = 0; i < MAX_SERVERS; i++) {
        if (q->sv[i] == NULL) {   
            q->sv[i] = server;
            break;
        }
    } 

    pthread_mutex_unlock(p);
}

// N'appeller qu'apres avoir fini une partie !!
void remove_server(pthread_mutex_t *p, server_t *server, queue_t *q) {
    pthread_mutex_lock(p);

    for (int i = 0; i < MAX_SERVERS; i++) {
        if (q->cl[i] != NULL) { 
            if ((server_t *)q->cl[i] == server) {
                free(q->sv[i]);
                q->sv[i] = NULL;
            }
        }
    } 

    pthread_mutex_unlock(p);
}

server_t *find_server(pthread_mutex_t *p, int gamemode_flag, queue_t *q) {
    pthread_mutex_lock(p);
    // printf("looking for server..");

    for (int i = 0; i < MAX_SERVERS; i++) {
        // printf("visiting servers array [%d]\n", i);
        if (q->sv[i] != NULL) { 
        // printf("found server non null [%d]\n", i);
            if (q->sv[i]->gamemode == gamemode_flag && q->sv[i]->players_count < 4) {
                // printf("found server %d!\n",q->sv[i]->game_id);
                pthread_mutex_unlock(p);
                return q->sv[i];
            }
        }
    }

    printf("no server found..");

    // If no game is found, we create one
    server_t *new_server = malloc(sizeof(server_t));

    init_server(new_server, gamemode_flag);

    pthread_mutex_unlock(p);
    add_server(p, new_server, q);

    
    return new_server;
}

server_t *sv_get(pthread_mutex_t *p, queue_t *q, int id) {
    pthread_mutex_lock(p);
    
    for (int i = 0; i < MAX_SERVERS; i++) {
        if (q->sv[i] != NULL && q->sv[i]->game_id == id) {
                pthread_mutex_unlock(p);
                return q->sv[i];
        }
    }

    pthread_mutex_unlock(p);
}

client_t *find_client(pthread_mutex_t *p, queue_t *q, int sock_fd) {
    pthread_mutex_lock(p);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (q->cl[i] != NULL) { 
            if (q->cl[i]->sock_fd == sock_fd) {
                pthread_mutex_unlock(p);
                return q->cl[i];
            }
        }
    } 


    pthread_mutex_unlock(p);
    return NULL;
}