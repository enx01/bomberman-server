#include "../../headers/structs.h"
#include <pthread.h>
#ifndef QUEUE_HANDLER_H
#define QUEUE_HANDLER_H

void init_queue(queue_t *);

void add_client(pthread_mutex_t *, client_t *client, queue_t *);
void remove_client(pthread_mutex_t *,client_t *client, queue_t *);
client_t *find_client(pthread_mutex_t *p, queue_t *q, int sock_fd);

void add_server(pthread_mutex_t *, server_t *, queue_t *);
void remove_server(pthread_mutex_t *, server_t *, queue_t *);
server_t *find_server(pthread_mutex_t *, int gamemode_flag, queue_t *);
server_t *sv_get(pthread_mutex_t *p, queue_t *q, int id);

#endif